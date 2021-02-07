#include <cerrno>
#include <sys/epoll.h>
#include "Epoll.h"
#include "Channel.h"
#include "TcpConnection.h"

using namespace ssxrver;
using namespace ssxrver::net;
enum
{
    kNew = -1,   //三种状态对应的是啥，刚构造了一个channel对象，初始化时是-1，还没有添加到epoll中关注
    kAdded = 1,  //在epollWait上关注
    kDeleted = 2 //删除
};

Epoll::Epoll(EventLoop *loop)
    : epollFd(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize),
      ownerLoop_(loop)
{
    if (epollFd < 0)
    {
        LOG_SYSFATAL << "Epoll::Epoll";
    }
}

Epoll::~Epoll()
{

     for (auto &item : connections_)
     {
         item.second.reset();
     }
    ::close(epollFd);
}

void Epoll::poll(ChannelList *activeChannels)
{
    int numEvents = ::epoll_wait(epollFd,
                                 &*events_.begin(), //事件动态数组，提前设好大小
                                 static_cast<int>(events_.size()),
                                 -1);
    int saveErrno = errno;
    if (numEvents > 0)
    {
         LOG_DEBUG << numEvents << "events happened";
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

void Epoll::fillActiveChannels(int numEvents,                     //返回活跃的事件个数
                                 ChannelList *activeChannels) const //返回活跃的事件
{
    for (int i = 0; i < numEvents; i++)
    {
        auto *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->setRevents(events_[i].events); //把返回的事件写到这个通道里
        activeChannels->push_back(channel);      //继而返回到eventloop中
    }
}

void Epoll::updateChannel(Channel *channel)
{
    ownerLoop_->assertInLoopThread();
    const int status_ = channel->status();
    if (status_ == kNew || status_ == kDeleted)
    {
        int fd = channel->fd(); //如果是新的通道，取他的fd值
        if (status_ == kNew)
        {
//            channels_.insert({fd,channel}); //新的，就加到关注队列
            channels_[fd] = channel;
        }
        channel->setStatus(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->setStatus(kDeleted); //这里状态需要改变，处理kDeleted，仅仅表示是没有在epoll中关注，并不表示从channelmap中移除了，想要在次关注，执行上面代码
        }
        else
            update(EPOLL_CTL_MOD, channel); //修改这个通道
    }
}

void Epoll::removeChannel(Channel *channel)
{
    ownerLoop_->assertInLoopThread();
    int fd = channel->fd();
    int status_ = channel->status();
    if (connections_.count(fd) != 0) {
        connectionsPool.emplace_back(std::move(connections_[fd]));
//        connections_.erase(fd);
        ::close(fd);
    }
    else {
        LOG_FATAL << "erase connections_";
    }

    if (status_ == kAdded)
        update(EPOLL_CTL_DEL, channel);
    channel->setStatus(kNew);
}

void Epoll::update(int operation, Channel *channel) const
{
    struct epoll_event event{}; //准备一个epoll_event
    bzero(&event, sizeof(event));
    event.events = channel->events(); //关注这个事件
    event.data.ptr = channel;
    int fd = channel->fd();
    if (::epoll_ctl(epollFd, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
            LOG_SYSERR << "epoll_ctl op = " << operation << " fd = " << fd; //添加失败，不会退出程序
        else
            LOG_SYSFATAL << "epoll_ctl op = " << operation << "fd = " << fd; //其他错误直接结束程序
    }
}

void Epoll::createConnection(int sockFd, const ConnectionCallback &connectCallback,
                             const MessageCallback &messageCallback, const WriteCompleteCallback &writeCompleteCallback)
{
    if(!connectionsPool.empty()){
        connectionsPool.back()->connectReset(sockFd);
        connections_[sockFd] = std::move(connectionsPool.back());
        connectionsPool.pop_back();
        ownerLoop_->runInLoop([&conn = connections_[sockFd]] { conn->connectEstablished(); });
    }
    else {
        TcpConnectionPtr conn = std::make_shared<TcpConnection>(ownerLoop_, //所属ioLoop
                                                                sockFd);
        conn->setConnectionCallback(connectCallback);
        conn->setMessageCallback(messageCallback);
        conn->setWriteCompleteCallback(writeCompleteCallback);
        conn->setCloseCallback(
                [this](auto && PH1) { removeConnection(PH1); });
//        conn->getChannel()->tie(conn);
        connections_[sockFd] = std::move(conn);
        ownerLoop_->runInLoop([&conn = connections_[sockFd]] { conn->connectEstablished(); });
    }
}

void Epoll::removeConnection(const TcpConnectionPtr &conn)
{
    ownerLoop_->runInLoop(
            [&conn] { conn->connectDestroyed(); });
}

