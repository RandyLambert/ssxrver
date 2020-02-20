#ifndef SSXRVER_NET_EPOLLER_H
#define SSXRVER_NET_EPOLLER_H
#include <vector>
#include <map>
#include "../base/noncopyable.h"
#include "EventLoop.h"

struct epoll_event;
namespace ssxrver
{
namespace net
{
class Channel;
class Epoller : noncopyable
{
public:
    typedef std::vector<Channel *> ChannelList;
    Epoller(EventLoop *loop);
    ~Epoller();
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    void poll(ChannelList *activeChannels);

private:
    static const int kInitEventListSize = 16;
    static const int timeoutMs = -1;

    void fillActiveChannels(int numEvents,ChannelList *activeChannels) const;
    void update(int operation,Channel *channel);

    typedef std::map<int, Channel *> ChannelMap; //fd和事件指针
    ChannelMap channels_;                        //监听检测通道
    int epollfd_;
    typedef std::vector<struct epoll_event> EventList;
    EventList events_; //实际返回的事件列表
    EventLoop *loop_;

};

}
}
#endif
