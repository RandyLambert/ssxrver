#ifndef SSXRVER_NET_TCPCONNECTION_H
#define SSXRVER_NET_TCPCONNECTION_H
#include <memory>
#include <boost/any.hpp>
#include <netinet/in.h>
#include <functional>
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
    typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;
    typedef std::function<void(const TcpConnectionPtr &,
                               Buffer *)>
        MessageCallback;
    typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
    typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;
    TcpConnection(EventLoop *loop,
                  int socked);
    ~TcpConnection();

    EventLoop *getLoop() const { return loop_; }

    bool connected() const { return state_ == kConnected; }
    void send(string &&message); // C++11
    void send(const void *message, int len);
    void send(const char *message);
    void send(const string &message);
    void send(Buffer &&message); // C++11
    void send(Buffer *message);  // this one will swap data
    void shutdown();
    void forceClose();
    void setTcpNoDelay(bool on);
    void startRead();
    void stopRead();
    bool isReading() const { return reading_; }

    void setContext(const boost::any &context) //把一个未知类型赋值
    {
        context_ = context;
    }

    const boost::any &getContext() const //获取未知类型，不能更改
    {
        return context_;
    }

    boost::any *getMutableContext() //get可变的，可以更改
    {
        return &context_;
    }
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
    int returnSockfd() const { return sockfd_; }
    std::unique_ptr<Channel>& getChannel(){ return channel_; }

    Buffer *inputBuffer() { return &inputBuffer_; }
    Buffer *outputBuffer() { return &outputBuffer_; }
    void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }

    void connectEstablished();
    void connectDestroyed();

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

    void sendInLoop(const string &message);
    void sendInLoop(const char *message);
    void sendInLoop(const void *message, size_t len);
    void shutdownInLoop();
    void forceCloseInLoop();

    void setState(StateE s) { state_ = s; }
    void startReadInLoop();
    void stopReadInLoop();

    EventLoop *loop_; //所属eventloop
    StateE state_;    //FIXME atomic
    int sockfd_;
/* public: */
    std::unique_ptr<Channel> channel_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    CloseCallback closeCallback_;

    Buffer inputBuffer_;  //应用层的接收缓冲区
    Buffer outputBuffer_; //应用层的发送缓冲区，当outputbuffer高到一定程度
    boost::any context_;  //提供一个借口绑定位置类型的上线文对象
    bool reading_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
void defaultMessageCallback(const TcpConnectionPtr &conn,
                            Buffer *buffer);
void defaultConnectionCallback(const TcpConnectionPtr &conn);
} // namespace net
} // namespace ssxrver

#endif
