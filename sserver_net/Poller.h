// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef SSERVER_POLLER_H
#define SSERVER_POLLER_H

#include <map>
#include <vector>
#include "../sserver_base/Timestamp.h"
#include "EventLoop.h"

namespace sserver
{
namespace net
{

class Channel;

///
/// Base class for IO Multiplexing
///
/// This class doesn't own the Channel objects.
class Poller
{
public:
  typedef std::vector<Channel *> ChannelList;

  Poller(const Poller &) = delete;
  Poller &operator=(const Poller &) = delete;
  Poller(EventLoop *loop);
  virtual ~Poller();

  /// Polls the I/O events.
  /// Must be called in the loop thread.
  virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;

  /// Changes the interested I/O events.
  /// Must be called in the loop thread.
  virtual void updateChannel(Channel *channel) = 0; //三个纯虚函数

  /// Remove the channel, when it destructs.
  /// Must be called in the loop thread.
  virtual void removeChannel(Channel *channel) = 0;

  virtual bool hasChannel(Channel *channel) const;

  static Poller *newDefaultPoller(EventLoop *loop);

  void assertInLoopThread() const
  {
    ownerLoop_->assertInLoopThread();
  }

protected:
  typedef std::map<int, Channel *> ChannelMap;
  ChannelMap channels_; //监听检测通道

private:
  EventLoop *ownerLoop_; //poller所属的eventloop
};

} // namespace net
} // namespace sserver
#endif // SSERVER_POLLER_H
