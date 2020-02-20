#ifndef SSXRVER_NET_TCPCONNECTION_H
#define SSXRVER_NET_TCPCONNECTION_H
#include <memory>
#include <any>
#include "functional"
#include "Buffer.h"
#include "../base/noncopyable.h"
namespace ssxrver
{
namespace net
{
class EventLoop;
class Channel;

class TcpConnection : noncopyable,
                     public std::enable_shared_from_this<TcpConnection>
{
public:
    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
    typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
    typedef std::function<void(const TcpConnectionPtr &)> MessageCallback;
    typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;
    TcpConnection(EventLoop *loop,
                  int socked); 
    ~TcpConnection();

    EventLoop *getLoop() const { return loop_; }

private:
    enum StateE //连接的状态
    {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const char *message);
    void sendInLoop(const void *message, size_t len);
    void shutdownInLoop();

    void setState(StateE s){ state_ = s; }

    StateE state_;
    std::unique_ptr<Channel> channel_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    EventLoop *loop_;        //所属eventloop
    Buffer inputBuffer_;   //应用层的接收缓冲区
    Buffer outputBuffer_;  //应用层的发送缓冲区，当outputbuffer高到一定程度
    std::any context_;     //提供一个接口绑定一个未知类型的上下文对象，我们不清楚上层的网络程序会绑定一个什么对象，提供这样的接口，帮助应用程序
    bool reading_;

};
}
}

#endif
