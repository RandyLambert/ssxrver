#include <fcntl.h>
#include <boost/get_pointer.hpp>
#include <cassert>
#include "TcpServer.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "SocketOps.h"
#include "../util/Init.h"

using namespace ssxrver;
using namespace ssxrver::net;

TcpServer::TcpServer(EventLoop *loop,
                     struct sockaddr_in listenAddr) //在main函数初始化
    : loop_(loop),
      threadPool_(std::make_unique<EventLoopThreadPool>(loop)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback),
      started_(false),
      acceptFd_(socketops::createNonblockingOrDie()),
      idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
      acceptChannel_(Channel(loop, acceptFd_))
{
    socketops::setReuseAddr(acceptFd_, true);
    socketops::setReusePort(acceptFd_, true);
    socketops::bindOrDie(acceptFd_, &listenAddr);
    acceptChannel_.setReadCallback(
        [this] { acceptHandRead(); });
}

TcpServer::~TcpServer()
{
    loop_->assertInLoopThread();
    ::close(idleFd_);
}

void TcpServer::setThreadNum(int numThreads)
{
    assert(0 <= numThreads);
    threadPool_->setThreadNum(numThreads);
}

//该函数可以跨线程调用
void TcpServer::acceptSocketListen()
{
    socketops::listenOrDie(acceptFd_);
    acceptChannel_.enableEvents(kReadEventET);
//    acceptChannel_.enableEvents(kReadEventLT);
//    strace -o et.txt trace=all -p 28979
}

void TcpServer::acceptHandRead()
{
    loop_->assertInLoopThread();
    struct sockaddr_in peerAddr{};
    bzero(&peerAddr, sizeof peerAddr);

    int connFd;
    while (true)
    {
        connFd = socketops::accept(acceptFd_, &peerAddr);
        if (connFd >= 0) //得到了一个连接
        {
            LOG_DEBUG << "accept success" << inet_ntoa(peerAddr.sin_addr);
            if(ssxrver::util::Init::getInstance().getBlocksIp().count(inet_ntoa(peerAddr.sin_addr)) == 0 &&
                    (ssxrver::util::Init::getInstance().getWorkerConnections() == -1 || ssxrver::util::Init::getInstance().getWorkerConnections() >= connFd))
                newConnection(connFd);
            else
                ::close(connFd);
        }
        else
        {
            if(errno == EAGAIN) //ET模式读完了
                break;

            if (errno == EMFILE) //文件描述符太多了
            {
                LOG_SYSERR << "in TcpServer Accepteror";
                ::close(idleFd_);                        //先关闭开始的空闲文件描述符
                idleFd_ = accept(acceptFd_, nullptr, nullptr); //用这个描述符先接收
                ::close(idleFd_);                        //接收完在关闭，因为使用的是LT模式，不然accept会一直触发
                idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
            }
        }
    }
}

void TcpServer::start() //这个函数就是的Acceptor处于监听状态
{
    if (!started_)
    {
        started_ = true;
        threadPool_->start(threadInitCallback_);
        loop_->runInLoop([this] { acceptSocketListen(); });
    }
}

void TcpServer::newConnection(int sockFd)
{
    loop_->assertInLoopThread(); //断言在io线程
    //按照轮叫的方式选择一个eventLoop，将这个歌新连接交给这个eventLoop
    EventLoop *ioLoop = threadPool_->getNextLoop(); //选出这个io线程
    ioLoop->queueInLoop([ioLoop, sockFd, this] { ioLoop->createConnection(sockFd, connectionCallback_, messageCallback_, writeCompleteCallback_); });
}