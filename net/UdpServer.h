//
// Created by randylambert on 2021/2/15.
//

#ifndef SSXRVER_NET_UDPSERVER_H
#define SSXRVER_NET_UDPSERVER_H
#include <map>
#include <memory>
#include <functional>
#include <atomic>
#include <boost/noncopyable.hpp>
#include "TcpConnection.h"
#include "Channel.h"
namespace ssxrver::net {

class EventLoop;

class EventLoopThreadPool;

class UdpServer : boost::noncopyable
{
public:
    explicit UdpServer(EventLoop *loop,
                       struct sockaddr_in listenAddr);
    using UdpMessageCallback = std::function<void (Buffer*,const Buffer*)>;
    ~UdpServer();

    [[nodiscard]] EventLoop *getLoop() const { return loop_; }
    void start(); //启动线程池
    void setMessageCallback(UdpMessageCallback cb) { messageCallback_ = std::move(cb); }
    void udpSocketListen();

private:
    void udpHandRead();
    void udpSend(sockaddr_in peer,const Buffer* recvBuf);
    EventLoop *loop_;
    UdpMessageCallback messageCallback_;
    std::atomic<bool> started_;
    int udpFd_;
    Channel udpChannel_;
    static const int kUdpPacketSize = 65535;
};

}
#endif //SSXRVER_UDPSERVER_H
