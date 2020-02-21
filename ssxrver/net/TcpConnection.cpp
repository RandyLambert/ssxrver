#include <errno.h>
#include <utility>
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

TcpConnection::TcpConnection(EventLoop *loop,
                             int sockfd)
    :loop_(loop),
    state_(kConnected),
    sockfd_(sockfd),
    channel_(new Channel(loop,sockfd)),
    reading_(true)
{
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead,this));
    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite,this));
    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose,this));
    channel_->setErrorCallback(
        std::bind(&TcpConnection::handleError,this));
    LOG_DEBUG << "TcpConnection::ctor at"<<this<<"fd = "<<sockfd_;
    socketops::setKeepAlive(sockfd_,true);
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor at"<<this<<"fd = "<<sockfd_;
    assert(state_ == kDisconnected);
}





//send






void TcpConnection::shutdown()
{
    if(state_ == kConnected)
    {
        setState(kDisconnected);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop,shared_from_this()));
    }
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if(!channel_->isWriting())
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
    socketops::setTcpNoDelay(sockfd_,on);
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
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop,this));
}

void TcpConnection::stopReadInLoop()
{
    loop_->assertInLoopThread();
    if(reading_ || channel_->isReading())
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
    channel_->tie(shared_from_this());
    channel_->enableReading();
    LOG_TRACE << sockfd_ << " is "
              << (connected() ? "UP" : "DOWN");
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    if(state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();
        LOG_TRACE << sockfd_ << " is "
                  << (connected() ? "UP" : "DOWN");
    }
    channel_->remove();
}

void TcpConnection::handleRead()
{
    loop_->assertInLoopThread();
    int saveErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(),&saveErrno);
    if(n > 0)
    {
        messageCallback_(shared_from_this(),&inputBuffer_);
    }
    else if(n == 0)
    {
        handleClose();
    }
    else
    {
        errno = saveErrno;
        LOG_SYSERR <<"TcpConnection::handleRead";
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if(channel_->isWriting())
    {

    }
}
