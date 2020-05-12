#include <assert.h>
#include <errno.h>
#include <sys/epoll.h>
#include <poll.h>
#include "Epoller.h"
#include "../base/Logging.h"
#include "Channel.h"
using namespace ssxrver;
using namespace ssxrver::net;
enum
{
    kNew = -1,   //三种状态对应的是啥，刚构造了一个channel对象，初始化时是-1，还没有添加到epoll中关注
    kAdded = 1,  //在epollwait上关注
    kDeleted = 2 //删除
};

Epoller::Epoller(EventLoop *loop)
    : epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize),
      ownerLoop_(loop)
{
    if (epollfd_ < 0)
    {
        LOG_SYSFATAL << "EPollPoller::EpollPoller";
    }
}

Epoller::~Epoller()
{
    close(epollfd_);
}

void Epoller::poll(ChannelList *activeChannels)
{
    int numEvents = ::epoll_wait(epollfd_,
                                 &*events_.begin(), //事件动态数组，提前设好大小
                                 static_cast<int>(events_.size()),
                                 -1);
    int saveErrno = errno;
    if (numEvents > 0)
    {
        // LOG_INFO << numEvents << "events happned";
        fillActiveChannels(numEvents, activeChannels);
        if (static_cast<size_t>(numEvents) == events_.size()) //随着关注的事件个数逐渐增加
        {
            events_.resize(events_.size() * 2); //两倍扩容
        }
    }
    else
    {
        if (saveErrno != EINTR)
        {
            errno = saveErrno;
            LOG_SYSERR << "epoll::poll";
        }
    }
}

void Epoller::fillActiveChannels(int numEvents,                     //返回活跃的事件个数
                                 ChannelList *activeChannels) const //返回活跃的事件
{
    for (int i = 0; i < numEvents; i++)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->set_revents(events_[i].events); //把返回的事件写到这个通道里
        activeChannels->push_back(channel);      //继而返回到eventloop中
    }
}

void Epoller::updateChannel(Channel *channel)
{
    ownerLoop_->assertInLoopThread();
    const int status_ = channel->status();
    if (status_ == kNew || status_ == kDeleted)
    {
        int fd = channel->fd(); //如果是新的通道，取他的fd值
        if (status_ == kNew)
        {
            channels_[fd] = channel; //新的，就加到关注队列
            /* connections_.push_back(make_pair(fd,channel->getTie())); */
            connections_[fd] = channel->getTie();

        }
        channel->set_status(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        int fd = channel->fd();
        (void)fd;
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_status(kDeleted); //这里状态需要改变，处理kdeleted，仅仅表示是没有在epoll中关注，并不表示从channelmap中移除了，想要在次关注，执行上面代码
        }
        else
            update(EPOLL_CTL_MOD, channel); //修改这个通道
    }
}

void Epoller::removeChannel(Channel *channel)
{
    ownerLoop_->assertInLoopThread();
    int fd = channel->fd();
    int status_ = channel->status();
    /* LOG_INFO << channel->name_; */
    if (channels_.erase(fd) != true)
        LOG_FATAL << "erase channel";
    if (connections_.erase(fd) != true)
        LOG_FATAL << "erase connections_";

    if (status_ == kAdded)
        update(EPOLL_CTL_DEL, channel);
    channel->set_status(kNew);
}

void Epoller::update(int operation, Channel *channel)
{
    struct epoll_event event; //准备一个epoll_event
    bzero(&event, sizeof(event));
    event.events = channel->events(); //关注这个事件
    event.data.ptr = channel;
    int fd = channel->fd();
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
            LOG_SYSERR << "epoll_ctl op=" << operation << " fd=" << fd; //添加失败，不会退出程序
        else
            LOG_SYSFATAL << "epoll_ctl op=" << operation << "fd=" << fd; //其他错误直接结束程序
    }
}
