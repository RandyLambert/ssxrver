// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/sserver/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "EPollPoller.h"
#include "../../sserver_base/Logging.h"
#include "../Channel.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>

using namespace sserver;
using namespace sserver::net;

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
static_assert(EPOLLIN == POLLIN, "epoll");
static_assert(EPOLLPRI == POLLPRI, "epoll");
static_assert(EPOLLOUT == POLLOUT, "epoll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll");
static_assert(EPOLLERR == POLLERR, "epoll");
static_assert(EPOLLHUP == POLLHUP, "epoll");

namespace
{
const int kNew = -1; //三种状态对应是啥，刚构造一个channel对象，初始化时是-1，还没有添加到poll或epoll中被关注
const int kAdded = 1;
const int kDeleted = 2;
} // namespace

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize)
{
  if (epollfd_ < 0)
  {
    LOG_SYSFATAL << "EPollPoller::EPollPoller";
  }
}

EPollPoller::~EPollPoller()
{
  ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
  int numEvents = ::epoll_wait(epollfd_,
                               &*events_.begin(), //事件动态数组，提前设好大小
                               static_cast<int>(events_.size()),
                               timeoutMs);
  int savedErrno = errno;
  Timestamp now(Timestamp::now());
  if (numEvents > 0) //如果发生了事件，事件个数大与0
  {
    LOG_TRACE << numEvents << " events happended";
    fillActiveChannels(numEvents, activeChannels);
    if (static_cast<size_t>(numEvents) == events_.size()) //随着关注的事件个数逐渐增加
    {
      events_.resize(events_.size() * 2); //两倍扩容
    }
  }
  else if (numEvents == 0) //没发生时间，超时返回
  {
    LOG_TRACE << " nothing happended"; //日志记录
  }
  else
  {
    // error happens, log uncommon ones
    if (savedErrno != EINTR)
    {
      errno = savedErrno;
      LOG_SYSERR << "EPollPoller::poll()";
    }
  }
  return now;
}

void EPollPoller::fillActiveChannels(int numEvents,                     //返回事件个数
                                     ChannelList *activeChannels) const //把返回的事件放到这个事件列表中
{
  assert(static_cast<size_t>(numEvents) <= events_.size());
  for (int i = 0; i < numEvents; ++i)
  {
    Channel *channel = static_cast<Channel *>(events_[i].data.ptr); //把实际的channel已经保存到events.data.ptr中了
#ifndef NDEBUG
    int fd = channel->fd(); //dubug模式下的事情
    ChannelMap::const_iterator it = channels_.find(fd);
    assert(it != channels_.end());
    assert(it->second == channel);
#endif
    channel->set_revents(events_[i].events); //把返回的事件写到这个通道里
    activeChannels->push_back(channel);      //进而返回到eventloop中
  }
}

void EPollPoller::updateChannel(Channel *channel) //从channel的update开始调用eventloop的updata，最后调用这个的update
{
  Poller::assertInLoopThread();                                               //断言处于io线程
  LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events(); //调试
  const int index = channel->index();                                         //index有三种状态
  if (index == kNew || index == kDeleted)                                     //这两个，说明现在是在新的通道，表是要新增通道
  {
    // a new one, add with EPOLL_CTL_ADD
    int fd = channel->fd(); //如果是新的通道，取他的fd值
    if (index == kNew)
    {
      assert(channels_.find(fd) == channels_.end()); //断言是找不到的
      channels_[fd] = channel;                       //把这个通道添加到检测通道
    }
    else // index == kDeleted
    {
      assert(channels_.find(fd) != channels_.end()); //断言是已经添加的
      assert(channels_[fd] == channel);              //需要重新添加关注
    }
    channel->set_index(kAdded);     //表示这个状态是已经添加的状态
    update(EPOLL_CTL_ADD, channel); //既然是已经添加的状态，就要把他添加到epoll中
  }
  else
  {
    // update existing one with EPOLL_CTL_MOD/DEL
    int fd = channel->fd();
    (void)fd;
    assert(channels_.find(fd) != channels_.end()); //断言fd不等于end
    assert(channels_[fd] == channel);
    assert(index == kAdded);
    if (channel->isNoneEvent()) //剔除epoll事件
    {
      update(EPOLL_CTL_DEL, channel); //del
      channel->set_index(kDeleted);   //这里状态需要改变，处理kdeleted，仅仅表示是没有在epoll中关注，并不表示从channelmap中移除了，想要在次关注，执行上面代码
    }
    else
    {
      update(EPOLL_CTL_MOD, channel); //否则是更新通道，更新这个通道，这里状态是不变的
    }
  }
}

void EPollPoller::removeChannel(Channel *channel)
{
  Poller::assertInLoopThread();
  int fd = channel->fd();
  LOG_TRACE << "fd = " << fd;
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->isNoneEvent()); //断言已经没有要帮助的时间
  int index = channel->index();
  assert(index == kAdded || index == kDeleted);//等于断言他在通道中，如果是new，说明没在通道，没法删
  size_t n = channels_.erase(fd);//从map中移除
  (void)n;
  assert(n == 1);

  if (index == kAdded)//如果是kadded，还得从epoll的关注中移除
  {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->set_index(kNew);
}

void EPollPoller::update(int operation, Channel *channel)
{
  struct epoll_event event;         //准备一个epoll_event
  bzero(&event, sizeof event);      //清零
  event.events = channel->events(); //关注的是这个事件
  event.data.ptr = channel;
  int fd = channel->fd();
  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) //核心就是调用epoll_ctl
  {
    if (operation == EPOLL_CTL_DEL)
    {
      LOG_SYSERR << "epoll_ctl op=" << operation << " fd=" << fd; //添加失败，避免把程序退出
    }
    else
    {
      LOG_SYSFATAL << "epoll_ctl op=" << operation << " fd=" << fd; //其他失败，挂掉
    }
  }
}
