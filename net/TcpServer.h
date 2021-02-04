#ifndef SSXRVER_NET_TCPSERVER_H
#define SSXRVER_NET_TCPSERVER_H
#include <map>
#include <memory>
#include <functional>
#include <atomic>
#include <boost/noncopyable.hpp>
#include "TcpConnection.h"
#include "Channel.h"
#include "CallBacks.h"
namespace ssxrver::net
{

class EventLoop;
class EventLoopThreadPool;
class TcpServer : boost::noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    explicit TcpServer(EventLoop *loop,
              struct sockaddr_in listenAddr);
    ~TcpServer();

    [[nodiscard]] EventLoop *getLoop() const { return loop_; }
    void setThreadNum(int numThreads);

    void start(); //启动线程池
    void setThreadInitCallback(ThreadInitCallback cb) { threadInitCallback_ = std::move(cb); }
    void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }
    void setWriteCompleteCallback(WriteCompleteCallback cb) { writeCompleteCallback_ = std::move(cb); }
    void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }
    void acceptSockListen();

private:
    void newConnection(int sockFd);
    void acceptHandRead();

    EventLoop *loop_;
    std::unique_ptr<EventLoopThreadPool> threadPool_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;
    std::atomic<bool> started_;
    int acceptFd_;
    int idleFd_;
    Channel acceptChannel_;
};

} // namespace ssxrver::net
#endif
