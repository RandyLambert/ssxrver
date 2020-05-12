#include <errno.h>
#include <utility>
#include "../base/Logging.h"
#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "SocketOps.h"
using namespace ssxrver;
using namespace ssxrver::net;
void ssxrver::net::defaultConnectionCallback(const TcpConnectionPtr &conn)
{ //默认的连接到来函数，如果自己设置，是在tcpserver设置
    LOG_DEBUG << "TcpConnection::ctor at" << conn->returnSockfd();
}

void ssxrver::net::defaultMessageCallback(const TcpConnectionPtr &,
                                          Buffer *buf)
{ //消息到来函数，如果自己设置，是在tcpserver处设置
    buf->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop *loop,
                             int sockfd)
    : loop_(loop),
      state_(kConnected),
      sockfd_(sockfd),
      channel_(new Channel(loop, sockfd)),
      reading_(true)
{
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this));
    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(
        std::bind(&TcpConnection::handleError, this));
    /* channel_->name_ = "连接来的channel名"; */
    socketops::setKeepAlive(sockfd_, true);
    LOG_INFO << "TcpConnection::ctor at" << this << "fd = " << sockfd_;
}

TcpConnection::~TcpConnection()
{
    LOG_INFO << "TcpConnection::dtor at" << this << "fd = " << sockfd_;
    assert(state_ == kDisconnected);
}

void TcpConnection::send(const void *data, int len)
{
    send(string(static_cast<const char *>(data), len));
}

void TcpConnection::send(const char *data)
{
    string tp(data);
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(tp);
        }
    }
    else
    {
        void (TcpConnection::*fp)(const string &message) = &TcpConnection::sendInLoop;
        loop_->runInLoop(
            std::bind(fp,
                      this,
                      tp));
    }
}
void TcpConnection::send(string &&message)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
            sendInLoop(message);
    }
    else
    {
        void (TcpConnection::*fp)(const string &message) = &TcpConnection::sendInLoop;
        loop_->runInLoop(
            std::bind(fp,
                      this,
                      std::forward<string>(message)));
    }
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
            void (TcpConnection::*fp)(const string &message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(
                std::bind(fp,
                          this,
                          buf->retrieveAllAsString()));
        }
    }
}

//线程安全的，可以跨线程调用
void TcpConnection::send(Buffer &&buf)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(buf.peek(), buf.readableBytes());
            buf.retrieveAll(); //把缓冲区数据移除
        }
        else
        {
            void (TcpConnection::*fp)(const string &message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(
                std::bind(fp,
                          this,
                          buf.retrieveAllAsString()));
        }
    }
}

void TcpConnection::sendInLoop(const string &message)
{
    sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const char *message)
{
    sendInLoop(message, strlen(message));
}

void TcpConnection::sendInLoop(const void *data, size_t len)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len; //len是我们要发送的数据
    bool faultError = false;
    if (state_ == kDisconnected)
    {
        LOG_WARN << "disconnected ,give uo writing";
        return;
    }
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) //没有关注可写时间，且outputbuffer缓冲区没有数据
    {
        nwrote = socketops::write(channel_->fd(), data, len); //可以直接write
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            //写完了，回调writecompletecallback
            if (remaining == 0 && writeCompleteCallback_) //如果等于0，说明都发送完毕，都拷贝到了内核缓冲区
            {
                LOG_INFO << "SENDINLOOP";
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
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
        outputBuffer_.append(static_cast<const char *>(data) + nwrote, remaining);
        //然后后面还有(data) + nwrote的数据没发送，就把他添加到outputbuffer中
        if (!channel_->isWriting())
            channel_->enableWriting(); //关注epollout事件，等对等方接受了数据，tcp的滑动窗口滑动了，
                                       //这时内核的发送缓冲区有位置了，epollout事件被触发，会回调tcpconnection::handlewrite
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
        setState(kDisconnected);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
    }
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if (!channel_->isWriting())
        socketops::shutdownWrite(sockfd_);
}

void TcpConnection::forceClose()
{
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        setState(kDisconnecting);
        loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
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

void TcpConnection::setTcpNoDelay(bool on)
{
    socketops::setTcpNoDelay(sockfd_, on);
}

void TcpConnection::startRead()
{
    loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop()
{
    loop_->assertInLoopThread();
    if (!reading_ || !channel_->isReading())
    {
        channel_->enableReading();
        reading_ = true;
    }
}

void TcpConnection::stopRead()
{
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop()
{
    loop_->assertInLoopThread();
    if (reading_ || channel_->isReading())
    {
        channel_->disableReading();
        reading_ = false;
    }
}

void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();

    assert(state_ == kConnected);
    setState(kConnected);
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();
    }
    channel_->remove();
}

void TcpConnection::handleRead()
{
    /* LOG_INFO << "handleRead"; */
    loop_->assertInLoopThread();
    int saveErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &saveErrno);
    if (n > 0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_);
    }
    else if (n == 0)
    {
        /* LOG_INFO << "handleclose" << channel_->name_; */
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
        ssize_t n = socketops::write(channel_->fd(), //这是就把outputbuffer中写入
                                     outputBuffer_.peek(),
                                     outputBuffer_.readableBytes());
        if (n > 0) //不一定写完了，写了n个字节
        {
            outputBuffer_.retrieve(n);              //缓冲区下标的移动，因为这是已经写了n个字节了
            if (outputBuffer_.readableBytes() == 0) //==0说明发送缓冲区已经清空
            {
                channel_->disableWriting(); //停止关注了pollout时间，以免出现busy_loop
                if (writeCompleteCallback_) //回调writecomplatecallback
                {
                    //应用层发送缓冲区被清空，就回调writecomplatecallback
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }

                if (state_ == kDisconnecting)
                {
                    shutdownInLoop(); //关闭连接
                }
            }
        }
        else
            LOG_SYSERR << "TcpConnection::handleWrite"; //发生错误
    }
}

void TcpConnection::handleClose()
{
    /* LOG_INFO << "handleCloce"; */
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisconnecting);
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr guardThis(shared_from_this());
    // must be the last line
    closeCallback_(guardThis); //调用tcpserverremoveconnection
}

void TcpConnection::handleError()
{
    int err = socketops::getSocketError(channel_->fd());
    LOG_ERROR << "TcpConnection::handleError "
              << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}
