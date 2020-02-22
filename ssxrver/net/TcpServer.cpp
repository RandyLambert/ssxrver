#include <cstdio>
#include "TcpServer.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "SocketOps.h"
#include "../base/Logging.h"
#include <boost/get_pointer.hpp>
using namespace ssxrver;
using namespace ssxrver::net;

TcpServer::TcpServer(EventLoop *loop,
                     struct sockaddr_in listenAddr)
    :loop_(loop),
    threadPool_(new EventLoopThreadPool(loop)),
    messageCallback_(defaultMessageCallback),
    nextConnId_(1),
    acceptfd_(socketops::createNonblockingOrDie()),
    acceptChannel_(new Channel(loop,acceptfd_))
{
    socketops::setReuseAddr(acceptfd_,true);
    socketops::setReusePort(acceptfd_,true);
    socketops::bindOrDie(acceptfd_,listenAddr);
}

TcpServer::~TcpServer()
{
    loop_->assertInLoopThread();
    LOG_TRACE << "TcpServer::~TcpServer destructing";

    for(auto&item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed,conn));
        conn.reset();
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    assert(0 <= numThreads);
    threadPool_->setThreadNum(numThreads);
}

//该函数多次调用是无害的
//该函数可以跨线程调用
void TcpServer::acceptSockListen()
{
    socketops::listenOrDie(acceptfd_);
    acceptChannel_->enableReading();
}
void TcpServer::start() //这个函数就是的Acceptor处于监听状态
{
    if(started_.getAndSet(1) == 0)
    {
        loop_->runInLoop(std::bind(&TcpServer::acceptSockListen,this));
    }
}

void TcpServer::newConnection(int sockfd)
{
    loop_->assertInLoopThread(); //断言在io线程
    //按照轮叫的方式选择一个eventloop，将这个歌新连接交给这个eventloop
    EventLoop *ioLoop = threadPool_->getNextLoop(); //选出这个io线程
    ++nextConnId_;
    LOG_INFO << "TcpServer::newConnection"<<sockfd;
    TcpConnectionPtr conn(new TcpConnection(ioLoop,//所属ioloop
                                            sockfd));
    connections_[sockfd] = conn;
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
             std::bind(&TcpServer::removeConnectionInLoop,this,std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished,conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    loop_->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnectionInLoop " << conn->returnSockfd();
    size_t n = connections_.erase(conn->returnSockfd());
    (void)n;
    assert(n == 1);
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn));
}
