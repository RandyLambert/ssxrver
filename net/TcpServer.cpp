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
      messageCallback_(defaultMessageCallback),
      started_(false),
      acceptFd(socketops::createNonblockingOrDie()),
      idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
      acceptChannel_(Channel(loop, acceptFd))
{
    socketops::setReuseAddr(acceptFd, true);
    socketops::setReusePort(acceptFd, true);
    socketops::bindOrDie(acceptFd, &listenAddr);
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
void TcpServer::acceptSockListen()
{
    socketops::listenOrDie(acceptFd);
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
        connFd = socketops::accept(acceptFd, &peerAddr);
        if (connFd >= 0) //得到了一个连接
        {
            LOG_DEBUG << "accept success" << inet_ntoa(peerAddr.sin_addr);
            if(ssxrver::util::Init::getInstance().getBlocksIp().count(inet_ntoa(peerAddr.sin_addr)) == 0 &&
                    (ssxrver::util::Init::getInstance().getWorkerConnections() == -1 || ssxrver::util::Init::getInstance().getWorkerConnections() >= connFd))
                newConnection1(connFd);
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
                idleFd_ = accept(acceptFd, nullptr, nullptr); //用这个描述符先接收
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
        loop_->runInLoop([this] { acceptSockListen(); });
    }
}

void TcpServer::newConnection(int sockFd)
{
    loop_->assertInLoopThread(); //断言在io线程
    //按照轮叫的方式选择一个eventLoop，将这个歌新连接交给这个eventLoop
    EventLoop *ioLoop = threadPool_->getNextLoop(); //选出这个io线程
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(ioLoop, //所属ioLoop
                                            sockFd);
//    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
        [this](auto && PH1) { removeConnection(PH1); });
    conn->getChannel()->tie(conn);
    LOG_INFO<<"newConnection"<<sockFd<<" "<<conn.use_count();
    ioLoop->queueInLoop([conn] { conn->connectEstablished(); });
}

void TcpServer::newConnection1(int sockFd)
{
    LOG_INFO<<"newConnection1 "<<sockFd;
    loop_->assertInLoopThread(); //断言在io线程
    //按照轮叫的方式选择一个eventLoop，将这个歌新连接交给这个eventLoop
    EventLoop *ioLoop = threadPool_->getNextLoop(); //选出这个io线程
    ioLoop->queueInLoop([ioLoop, sockFd, this] { ioLoop->createConnection(sockFd, connectionCallback_, messageCallback_, writeCompleteCallback_); });
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->runInLoop(
        [conn] { conn->connectDestroyed(); });
}
