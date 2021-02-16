//
// Created by randylambert on 2021/2/15.
//

#include "UdpServer.h"
#include "SocketOps.h"
#include "EventLoop.h"
using namespace ssxrver::net;
UdpServer::UdpServer(EventLoop *loop,
                     struct sockaddr_in listenAddr) //在main函数初始化
        : loop_(loop),
          messageCallback_(),
          started_(false),
          udpFd_(socketops::createDgramNonblockingOrDie()),
          udpChannel_(Channel(loop, udpFd_))
{
    socketops::setReuseAddr(udpFd_, true);
    socketops::setReusePort(udpFd_, true);
    socketops::bindOrDie(udpFd_, &listenAddr);
    udpChannel_.setReadCallback(
            [this] { udpHandRead(); });
}

UdpServer::~UdpServer()
{
    loop_->assertInLoopThread();
    ::close(udpFd_);
}

//该函数可以跨线程调用
void UdpServer::udpSocketListen()
{
    socketops::listenOrDie(udpFd_);
    udpChannel_.enableEvents(kReadEventET);
}

void UdpServer::udpHandRead()
{
    loop_->assertInLoopThread();
    struct sockaddr_in peerAddr{};
    bzero(&peerAddr, sizeof peerAddr);

    while(true)
    {
        std::shared_ptr<Buffer> buf = std::make_shared<Buffer>(kUdpPacketSize);
        ssize_t readn = ssxrver::net::socketops::recvfrom(udpFd_, buf->beginWrite(), buf->writeableBytes(), &peerAddr);
        if (readn < 0) {
            if(errno == EAGAIN) //ET模式读完了
                break;
            else
            {
                LOG_ERROR << "udp %d recv failed: " << udpFd_;
                continue;
            }
        }
        loop_->queueInLoop([this,peerAddr,recvBuf = std::move(buf)]{ udpSend(peerAddr,recvBuf.get());});
    }

}

void UdpServer::start() //这个函数就是的Acceptor处于监听状态
{
    if (!started_)
    {
        started_ = true;
        loop_->runInLoop([this] { udpSocketListen(); });
    }
}

void UdpServer::udpSend(sockaddr_in peer,const Buffer* recvBuf) {
    Buffer sendBuf(kUdpPacketSize);
    messageCallback_(&sendBuf,recvBuf);
    ssize_t writen = ssxrver::net::socketops::sendto(udpFd_, sendBuf.peek(), sendBuf.readableBytes(), &peer);
    if (writen < 0) {
        LOG_ERROR << "udp %d send failed: " << udpFd_;
    }
}
