#ifndef SSXRVER_NET_TCPSERVER_H
#define SSXRVER_NET_TCPSERVER_H
#include <map>
#include <memory>
#include <functional>
#include "../base/Atomic.h"
#include "../base/noncopyable.h"
#include "TcpConnection.h"
#include "EventLoop.h"
namespace ssxrver
{
namespace net
{

class EventLoop;
class EventLoopThreadPool;
using std::string;
class TcpServer : noncopyable
{
public:
    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
    typedef std::function<void()> TimerCallback;
    typedef std::function<void(EventLoop *)> ThreadInitCallback;
    typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
    typedef std::function<void(const TcpConnectionPtr &)> MessageCallback;
    typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;
    TcpServer(EventLoop *loop) {}
    ~TcpServer() {}

    void start(); //启动线程池
    void setConnectionCallback(ConnectionCallback cb){  connectionCallback_ = std::move(cb);}
    void setMessageCallback(MessageCallback cb){messageCallback_ = std::move(cb);}
    void setWriteCompleteCallback(WriteCompleteCallback cb){ writeCompleteCallback_ = std::move(cb); }
    

private:
    EventLoop *loop;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;       //io线程池中的线程在进入事件循环前，会调用此函数

};


}
}
#endif
