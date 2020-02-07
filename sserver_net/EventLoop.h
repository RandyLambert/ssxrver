// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/../
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef SSERVER_EVENTLOOP_H
#define SSERVER_EVENTLOOP_H

#include <vector>

#include <any>
#include <memory>
#include <functional>
#include "../sserver_base/Mutex.h"
#include "../sserver_base/CurrentThread.h"
#include "../sserver_base/Timestamp.h"
#include "Callbacks.h"
#include "TimerId.h"

namespace sserver
{
namespace net
{

class Channel;
class Poller;
class TimerQueue;
/*
one loop per thread的意思是说，每个线程都最多有一个eventloop对象
eventloop对象在构造的时候会检查当前线程是否已经创建了其他eventloop对象
如果已经创建，则终止程序（LOG_FATAL）
EventLoop构造函数会记住本对象的所属线程(threadld_)
创建了eventloop对象称为io线程，其功能是运行时间循环（eventloop::loop）
*/

///
/// Reactor, at most one per thread.
///
/// This is an interface class, so don't expose too much details.
class EventLoop //rector模式的封装
{
public:
  typedef std::function<void()> Functor;

  EventLoop();
  ~EventLoop(); // force out-line dtor, for scoped_ptr members.
  EventLoop(const EventLoop &) = delete;
  EventLoop &operator=(const EventLoop &) = delete;

  ///
  /// Loops forever.
  ///
  /// Must be called in the same thread as creation of the object.
  ///
  void loop();

  /// Quits loop.
  ///
  /// This is not 100% thread safe, if you call through a raw pointer,
  /// better to call through shared_ptr<EventLoop> for 100% safety.
  void quit();

  ///
  /// Time when poll returns, usually means data arrival.
  ///
  Timestamp pollReturnTime() const { return pollReturnTime_; }

  int64_t iteration() const { return iteration_; }

  /// Runs callback immediately in the loop thread.
  /// It wakes up the loop, and run the cb.
  /// If in the same loop thread, cb is run within the function.
  /// Safe to call from other threads.
  void runInLoop(const Functor &cb);
  /// Queues callback in the loop thread.
  /// Runs after finish pooling.
  /// Safe to call from other threads.
  void queueInLoop(const Functor &cb);

  void runInLoop(Functor &&cb);
  void queueInLoop(Functor &&cb);

  // timers

  ///
  /// Runs callback at 'time'.
  /// Safe to call from other threads.
  ///
  TimerId runAt(const Timestamp &time, const TimerCallback &cb);
  ///
  /// Runs callback after @c delay seconds.
  /// Safe to call from other threads.
  ///
  TimerId runAfter(double delay, const TimerCallback &cb);
  ///
  /// Runs callback every @c interval seconds.
  /// Safe to call from other threads.
  ///
  TimerId runEvery(double interval, const TimerCallback &cb);
  ///
  /// Cancels the timer.
  /// Safe to call from other threads.
  ///
  void cancel(TimerId timerId);

  TimerId runAt(const Timestamp &time, TimerCallback &&cb);
  TimerId runAfter(double delay, TimerCallback &&cb);
  TimerId runEvery(double interval, TimerCallback &&cb);

  // internal usage
  void wakeup();
  void updateChannel(Channel *channel); //在poller中添加或者更新通道
  void removeChannel(Channel *channel); //在poller中移除通道
  bool hasChannel(Channel *channel);

  // pid_t threadId() const { return threadId_; }
  void assertInLoopThread()
  {
    if (!isInLoopThread())
    {
      abortNotInLoopThread();
    }
  }
  bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
  // bool callingPendingFunctors() const { return callingPendingFunctors_; }
  bool eventHandling() const { return eventHandling_; }

  void setContext(const boost::any &context)
  {
    context_ = context;
  }

  const std::any &getContext() const
  {
    return context_;
  }

  std::any *getMutableContext()
  {
    return &context_;
  }

  static EventLoop *getEventLoopOfCurrentThread();

private:
  void abortNotInLoopThread(); //终止程序
  void handleRead();           // waked up
  void doPendingFunctors();

  void printActiveChannels() const; // DEBUG

  typedef std::vector<Channel *> ChannelList;

  bool looping_;                /* atomic */
  bool quit_;                   /* atomic and shared between threads, okay on x86, I guess. */
  bool eventHandling_;          /* atomic */
  bool callingPendingFunctors_; /* atomic */
  int64_t iteration_;
  const pid_t threadId_; //当前对象所属线程id
  Timestamp pollReturnTime_;
  std::unique_ptr<Poller> poller_;
  std::unique_ptr<TimerQueue> timerQueue_;
  int wakeupFd_;
  // unlike in TimerQueue, which is an internal class,
  // we don't expose Channel to client.
  std::unique_ptr<Channel> wakeupChannel_;
  std::any context_;

  // scratch variables
  ChannelList activeChannels_;    //poller返回的活动通道
  Channel *currentActiveChannel_; //当前正在处理的活动通道

  MutexLock mutex_;
  std::vector<Functor> pendingFunctors_; // @GuardedBy mutex_
};

} // namespace net
} // namespace sserver
#endif // SSERVER_EVENTLOOP_H
