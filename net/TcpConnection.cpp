#include <cerrno>
#include "../base/Logging.h"
#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "SocketOps.h"
using namespace ssxrver;
using namespace ssxrver::net;

void ssxrver::net::defaultMessageCallback(const TcpConnectionPtr &,
                                          Buffer *buf)
{ //消息到来函数，如果自己设置，是在tcpserver处设置
    buf->retrieveAll();
}

void ssxrver::net::defaultConnectionCallback(const TcpConnectionPtr &conn)
{
    LOG_DEBUG << (conn->connected() ? "up" : "down");
}

TcpConnection::TcpConnection(EventLoop *loop,
                             int sockFd)
    : loop_(loop),
      state_(kConnecting),
      sockFd_(sockFd),
      channel_(new Channel(loop, sockFd)),
      sendFile_(),
      context_(new HttpRequestParser()),
      reading_(true)
{
    channel_->setReadCallback(
        [this] { handleRead(); });
    channel_->setWriteCallback(
        [this] { handleWrite(); });
    channel_->setCloseCallback(
        [this] { handleClose(); });
    channel_->setErrorCallback(
        [this] { handleError(); });
    socketops::setKeepAlive(sockFd_, true);
}

TcpConnection::~TcpConnection()
{
    assert(state_ == kDisconnected);
}


void TcpConnection::send(Buffer *buf)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll(); //把缓冲区数据移除
        }
        else
        {
            loop_->runInLoop([this,buf]{ sendInLoop(buf->retrieveAllAsString(),buf->readableBytes()); });
        }
    }
}

void TcpConnection::send(std::string_view message){
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
            sendInLoop(message,message.size());
    }
    else
    {
        loop_->runInLoop([this,message]{sendInLoop(message,message.size()); });
    }
}

void TcpConnection::sendInLoop(std::string_view data, size_t len)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len; //len是我们要发送的数据
    bool faultError = false;
    if (state_ == kDisconnected)
    {
        LOG_WARN << "disconnected ,give up writing";
        return;
    }
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) //没有关注可写时间，且outputbuffer缓冲区没有数据
    {
        nwrote = socketops::write(channel_->fd(), data.begin(), len); //可以直接write
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            // 写完了，回调writeCompleteCallback
            LOG_DEBUG << "sendInLoop";
            if(!sendFile_)
            {
                if (remaining == 0 && writeCompleteCallback_) //如果等于0，说明都发送完毕，都拷贝到了内核缓冲区
                {
                    loop_->queueInLoop([ptr = shared_from_this()]{ ptr->writeCompleteCallback_(ptr);} );
                }
            } else {
                if (remaining == 0 && writeCompleteCallback_ && sendFile_->getSendLen() == 0) //如果等于0，说明都发送完毕，都拷贝到了内核缓冲区
                {
                    loop_->queueInLoop([ptr = shared_from_this()]{ ptr->writeCompleteCallback_(ptr);} );
                }
            }
        }
        else //nwrote < 0，出错了
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_SYSERR << "TcpConnection::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = true;
                }
            }
        }
    }
    assert(remaining <= len);
    //没有错误，并且还有未写完的数据(说明内核发送缓冲区满，要将未写完的数据添加到output buffer中)
    if (!faultError && remaining > 0)
    {
        outputBuffer_.append(data.begin() + nwrote, remaining);
        // 然后后面还有(data) + nwrote的数据没发送，就把他添加到outputbuffer中
        if (!channel_->isWriting())
            channel_->enableEvents(kWriteEvent); // 关注epollout事件，等对等方接受了数据，tcp的滑动窗口滑动了，
                                       // 这时内核的发送缓冲区有位置了，epollout事件被触发，会回调tcpconnection::handlewrite
    } else if(sendFile_) { // 发完头信息,后面还有文件
        remaining = sendFile_->getSendLen();
        faultError = false;
        nwrote = socketops::sendfile(channel_->fd(), sendFile_->getInId(), sendFile_->getOffset(), remaining); //可以直接write
        if (nwrote >= 0)
        {
            sendFile_->getSendLen() -= nwrote;
            remaining = sendFile_->getSendLen();
            *sendFile_->getOffset() += nwrote;
            //写完了，回调writeCompleteCallback
            if (remaining == 0 && writeCompleteCallback_) //如果等于0，说明都发送完毕，都拷贝到了内核缓冲区
            {
                LOG_DEBUG << "sendInLoop";
                sendFileReset();
                loop_->queueInLoop([ptr = shared_from_this()]{ptr->writeCompleteCallback_(ptr);});;
            }
        }
        else //nwrote < 0，出错了
        {
            if (errno != EWOULDBLOCK)
            {
                LOG_SYSERR << "TcpConnection::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = true;
                }
            }
        }
        assert(remaining <= sendFile_->getSendLen());
        //没有错误，并且还有未写完的数据(说明内核发送缓冲区满，要将未写完的数据添加到output buffer中)
        if (!faultError && remaining > 0)
        {
            LOG_DEBUG << "isSendFile_";
            if (!channel_->isWriting())
                channel_->enableEvents(kWriteEvent); //关注epollout事件，等对等方接受了数据，tcp的滑动窗口滑动了，
            //这时内核的发送缓冲区有位置了，epollout事件被触发，会回调tcpconnection::handlewrite
        }
    }
}

void TcpConnection::shutdown()
{
    //应用程序想关闭连接，但是有可能正处于发送数据的过程中，output buffer中有数据还没发送完，不能调用close()
    //保证conn->send(buff);只要网络没有故障，保证必须发到对端
    //conn->shutdown();如果想要关闭，必须判断是否有没法删的数据，如果有不应该直接关闭

    //不可以跨线程调用
    if (state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->runInLoop([capture0 = shared_from_this()] { capture0->shutdownInLoop(); });
    }
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if (!channel_->isWriting())
        socketops::shutdownWrite(sockFd_);
}

void TcpConnection::forceClose()
{
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        setState(kDisconnecting);
        loop_->queueInLoop([capture0 = shared_from_this()] { capture0->forceCloseInLoop(); });
    }
}

void TcpConnection::forceCloseInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        handleClose();
    }
}

void TcpConnection::setTcpNoDelay(bool on) const
{
    socketops::setTcpNoDelay(sockFd_, on);
}

void TcpConnection::startRead()
{
    loop_->runInLoop([this] { startReadInLoop(); });
}

void TcpConnection::startReadInLoop()
{
    loop_->assertInLoopThread();
    if (!reading_ || !channel_->isReading())
    {
        channel_->enableEvents(kReadEventLT);
        reading_ = true;
    }
}

void TcpConnection::stopRead()
{
    loop_->runInLoop([this] { stopReadInLoop(); });
}

void TcpConnection::stopReadInLoop()
{
    loop_->assertInLoopThread();
    if (reading_ || channel_->isReading())
    {
        channel_->disableEvents(kReadEventLT);
        reading_ = false;
    }
}

void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->enableEvents(kReadEventLT);

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handleRead()
{
    LOG_DEBUG << "handleRead";
    loop_->assertInLoopThread();
    int saveErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &saveErrno);
    if (n > 0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_);
    }
    else if (n == 0)
    {
        handleClose();
    }
    else
    {
        errno = saveErrno;
        LOG_SYSERR << "TcpConnection::handleRead";
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if (channel_->isWriting()) //如果关注epollout时间
    {
        if(outputBuffer_.readableBytes() > 0)
        {
            ssize_t n = socketops::write(channel_->fd(), //这是就把outputbuffer中写入
                                         outputBuffer_.peek(),
                                         outputBuffer_.readableBytes());
            if (n > 0) //不一定写完了，写了n个字节
            {
                outputBuffer_.retrieve(n);              //缓冲区下标的移动，因为这是已经写了n个字节了
                if (outputBuffer_.readableBytes() == 0) //==0说明发送缓冲区已经清空
                {
                    channel_->disableEvents(kWriteEvent); //停止关注了pollout时间，以免出现busy_loop
                    if (writeCompleteCallback_) //回调writecomplatecallback
                    {
                        //应用层发送缓冲区被清空，就回调writecomplatecallback
                        loop_->queueInLoop([ptr = shared_from_this()]{ ptr->writeCompleteCallback_(ptr);});
                    }

                    if (state_ == kDisconnecting)
                    {
                        shutdownInLoop(); //关闭连接
                    }
                }
            }
            else
                LOG_SYSERR << "TcpConnection::handleWrite"; //发生错误
        } else if(sendFile_) {
            ssize_t n = socketops::sendfile(channel_->fd(), sendFile_->getInId(), sendFile_->getOffset(), sendFile_->getSendLen());
            if (n > 0) //不一定写完了，写了n个字节
            {
                sendFile_->getSendLen() -= n;              //缓冲区下标的移动，因为这是已经写了n个字节了
                *sendFile_->getOffset() += n;
                if (sendFile_->getSendLen() == 0) // ==0说明发送缓冲区已经清空
                {
                    sendFileReset();
                    channel_->disableEvents(kWriteEvent); //停止关注了pollOut时间，以免出现busy_loop
                    if (writeCompleteCallback_) //回调writeCompleteCallback
                    {
                        //发送缓冲区被清空，就回调WriteCompleteCallback
                        loop_->queueInLoop([ptr = shared_from_this()]{ ptr->writeCompleteCallback_(ptr); });
                    }

                    if (state_ == kDisconnecting)
                    {
                        shutdownInLoop(); //关闭连接
                    }
                }
            }
            else
                LOG_SYSERR << "TcpConnection::handleWrite " << sendFile_->getInId(); //发生错误
        }
    }
}

void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisconnecting);
    setState(kDisconnected);
    channel_->disableAll();

    closeCallback_(shared_from_this()); //调用tcpserverremoveconnection
}

void TcpConnection::handleError()
{
    int err = socketops::getSocketError(channel_->fd());
    if(err != 104) //忽略Connection reset by peer错误,不输出log
    {
        LOG_ERROR << "TcpConnection::handleError "
                  << "- SO_ERROR = " << err << " " << strerror_tl(err);
    }
}

void TcpConnection::connectReset(int sockFd)
{
    state_ = kConnecting;
    sockFd_ = sockFd;
    channel_->channelReset(sockFd);
    context_->reset();
    reading_ = true;
    inputBuffer_.retrieveAll();
    outputBuffer_.retrieveAll();
    sendFileReset();
    socketops::setKeepAlive(sockFd_, true);
}