// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/sserver/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef SSERVER_TIMERQUEUE_H
#define SSERVER_TIMERQUEUE_H

#include <set>
#include <vector>

#include "../sserver_base/Mutex.h"
#include "../sserver_base/Timestamp.h"
#include "Callbacks.h"
#include "Channel.h"

namespace sserver
{
namespace net
{

class EventLoop;
class Timer;
class TimerId;

///
/// A best efforts timer queue.
/// No guarantee that the callback will be on time.
///
class TimerQueue //相当于一个定时器的管理类， 内部维护了一个列表，定时器列表
{
  //timerqueue数据结构的选择，能快速的根据当前时间找到已到期的定时器，也要高效的添加和删除timer
  //因为可以使用二插搜索树，用map或者set
public:
  TimerQueue(EventLoop *loop); //一个TimerQueue属于一个eventloop对象，而eventloop对象在一个io线程中创建的
  TimerQueue(const TimerQueue &) = delete;
  TimerQueue &operator=(const TimerQueue &) = delete;
  ~TimerQueue();

  ///
  /// Schedules the callback to be run at given time,
  /// repeats if @c interval > 0.0.
  ///
  /// Must be thread safe. Usually be called from other threads.
  /// 一定是线程安全的，可以跨线程调用。通常情况下被其他线程跨线程调用
  TimerId addTimer(const TimerCallback &cb, //添加一个定时器，返回一个外部类Timeid，供外部使用
                   Timestamp when,          //外部可以调用取消一个定时器
                   double interval);
  TimerId addTimer(TimerCallback &&cb, //在实际使用时，不会直接调用addtimer而是会调用eventloop中的
                   Timestamp when,     //runat 在某个时刻运行定时器
                   double interval);   //runafter 过一段时间运行定时器
                                       //runevery 每隔一段时间运行定时器
                                       //cancel 取消定时器

  void cancel(TimerId timerId); //取消一个定时器

private:
  // FIXME: use unique_ptr<Timer> instead of raw pointers.
  // unique无法得到同一个对象的两个unique_ptr指针
  // 但可以进项移动构造和移动赋值操作，即所有权可以移动到另一个对象（而非拷贝构造）
  typedef std::pair<Timestamp, Timer *> Entry; //按照时间戳排序
  typedef std::set<Entry> TimerList;           //使用set不用map
  typedef std::pair<Timer *, int64_t> ActiveTimer;
  typedef std::set<ActiveTimer> ActiveTimerSet; //和第二行保存的是同样的东西

  //以下成员函数只可能在所属的i/o线程中调用，因而不必加锁，因为其他线程不会跨线程调用
  //锁竞争户导致服务器的性能大大降低，所以要尽量减少锁的使用
  void addTimerInLoop(Timer *timer);
  void cancelInLoop(TimerId timerId);
  // called when timerfd alarms
  void handleRead(); //回调函数
  // move out all expired timers
  std::vector<Entry> getExpired(Timestamp now);                 //返回超时的定时器列表，同一时间可能很多定时器返回
  void reset(const std::vector<Entry> &expired, Timestamp now); //对超时的定时器进行重置，因为超时的定时器可能是重复的，对定时器进行重置

  bool insert(Timer *timer); //插入定时器

  EventLoop *loop_; //所属的eventloop
  const int timerfd_;
  Channel timerfdChannel_; //定时器通道，在定时器事件到来后会回调handleread函数
  // Timer list sorted by expiration
  TimerList timers_; //timers_是按到期时间排序的

  // for cancel()
  //timers_与activeTimers_保存的是相同的数据
  //timers_是按照到期时间排序的，activeTimers_是按照对象地址排序的
  ActiveTimerSet activeTimers_;    //按照对象地址排序
  bool callingExpiredTimers_;      /* 是否正在处于调用处理超时定时器的atomic */
  ActiveTimerSet cancelingTimers_; //保护被取消的定时器
};

} // namespace net
} // namespace sserver
#endif // SSERVER_TIMERQUEUE_H
