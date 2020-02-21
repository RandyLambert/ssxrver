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
class TcpServer : noncopyable
{
public:
    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
    typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;
    typedef std::function<void(const TcpConnectionPtr &)> MessageCallback;
    typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;
    TcpServer(EventLoop *loop,
              struct sockaddr_in listenAddr);
    ~TcpServer();

    EventLoop *getLoop() const {return loop_;}
    void setThreadNum(int numThreads);
    std::shared_ptr<EventLoopThreadPool> threadPool() { return threadPool_; }

    void start(); //启动线程池
    void setMessageCallback(MessageCallback cb){messageCallback_ = std::move(cb);}
    void setWriteCompleteCallback(WriteCompleteCallback cb){ writeCompleteCallback_ = std::move(cb); }
    

private:
    void newConnection(int socked,const struct sockaddr_in peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    typedef std::map<string,TcpConnectionPtr> ConnectionMap;

    EventLoop *loop_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    AtomicInt32 started_;
    int nextConnId_;//下一个连接id
    ConnectionMap connections_; //连接累彪保留这个服务器上的所有连接

};

}
}
#endif
