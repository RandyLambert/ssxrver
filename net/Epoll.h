#ifndef SSXRVER_NET_EPOLL_H
#define SSXRVER_NET_EPOLL_H
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <boost/noncopyable.hpp>
#include "EventLoop.h"
#include "CallBacks.h"

struct epoll_event;
namespace ssxrver::net
{

class Channel;
class TcpConnection;
class Epoll : boost::noncopyable
{
public:
    using ChannelVec = std::vector<Channel *>;
    explicit Epoll(EventLoop *loop);
    ~Epoll();
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    void poll(ChannelVec *activeChannels);
    void createConnection(int sockFd, const ConnectionCallback &connectCallback,
                          const MessageCallback &messageCallback, const WriteCompleteCallback &writeCompleteCallback);

private:
    enum
    {                // 三种状态对应的是啥
        kNew = -1,   // 刚构造了一个channel对象，初始化时是-1，还没有添加到epoll中关注
        kAdded = 1,  // 在epollWait上关注
        kDeleted = 2 // 删除
    };
    static const int kInitEventSize = 16;
    void removeConnection(const TcpConnectionPtr &conn);
    void fillActiveChannels(int numEvents, ChannelVec *activeChannels) const;
    void update(int operation, Channel *channel) const;

    using ChannelMap = std::unordered_map<int, Channel *>;                      //fd和事件指针
    using TcpConnMap = std::unordered_map<int, TcpConnectionPtr>; //TcpConnectionMap
    std::vector<TcpConnectionPtr> connectionsPool;
    ChannelMap channels_;                                             //监听检测通道
    TcpConnMap connections_;
    int epollFd;
    std::vector<struct epoll_event> events_; //实际返回的事件列表
    EventLoop *ownerLoop_;

};

} // namespace ssxrver::net
#endif
