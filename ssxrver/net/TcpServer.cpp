#include <cstdio>
#include "TcpServer.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "SocketOps.h"
#include "../base/Logging.h"
#include <fcntl.h>
#include <boost/get_pointer.hpp>
using namespace ssxrver;
using namespace ssxrver::net;

TcpServer::TcpServer(EventLoop *loop,
                     struct sockaddr_in listenAddr) //在main函数初始化
    : loop_(loop),
      threadPool_(new EventLoopThreadPool(loop)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback),
      started_(false),
      nextConnId_(1),
      acceptfd_(socketops::createNonblockingOrDie()),
      idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
      acceptChannel_(loop, acceptfd_)
{
    socketops::setReuseAddr(acceptfd_, true);
    socketops::setReusePort(acceptfd_, true);
    socketops::bindOrDie(acceptfd_, listenAddr);
    acceptChannel_.setReadCallback(
        std::bind(&TcpServer::acceptHandRead, this));
}

TcpServer::~TcpServer()
{
    loop_->assertInLoopThread();
    LOG_TRACE << "TcpServer::~TcpServer destructing";

    for (auto &item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn));
        conn.reset();
    }
    ::close(idleFd_);
}

void TcpServer::setThreadNum(int numThreads)
{
    assert(0 <= numThreads);
    threadPool_->setThreadNum(numThreads);
}

//该函数可以跨线程调用
void TcpServer::acceptSockListen()
{
    socketops::listenOrDie(acceptfd_);
    acceptChannel_.enableReading();
}

void TcpServer::acceptHandRead()
{
    loop_->assertInLoopThread();
    struct sockaddr_in peerAddr;
    bzero(&peerAddr, sizeof peerAddr);
    int connfd = socketops::accept(acceptfd_, &peerAddr);
    if (connfd >= 0) //得到了一个连接
    {
        LOG_INFO << "accept success" << inet_ntoa(peerAddr.sin_addr);
        newConnection(connfd);
    }
    else
    {
        LOG_SYSERR << "in TcpServer Accepteror";
        if (errno == EMFILE) //文件描述符太多了
        {
            ::close(idleFd_);                        //先关闭开始的空闲文件描述符
            idleFd_ = accept(acceptfd_, NULL, NULL); //用这个描述符先接收
            ::close(idleFd_);                        //接收完在关闭，因为使用的是lt模式，不然accept会一直触发
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

void TcpServer::start() //这个函数就是的Acceptor处于监听状态
{
    if (started_ == false)
    {
        started_ = true;
        threadPool_->start(threadInitCallback_);
        loop_->runInLoop(std::bind(&TcpServer::acceptSockListen, this));
    }
}

void TcpServer::newConnection(int sockfd)
{
    loop_->assertInLoopThread(); //断言在io线程
    //按照轮叫的方式选择一个eventloop，将这个歌新连接交给这个eventloop
    EventLoop *ioLoop = threadPool_->getNextLoop(); //选出这个io线程
    ++nextConnId_;
    LOG_INFO << "TcpServer::newConnection" << sockfd;
    TcpConnectionPtr conn(new TcpConnection(ioLoop, //所属ioloop
                                            sockfd));
    connections_[sockfd] = conn;
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setConnectionCallback(connectionCallback_);
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnectionInLoop, this, std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
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
