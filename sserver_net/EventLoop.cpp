// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/sserver/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "EventLoop.h"
#include "../sserver_base/Logging.h"
#include "../sserver_base/Mutex.h"
#include "Channel.h"
#include "Poller.h"
#include "SocketsOps.h"
#include "TimerQueue.h"

#include <signal.h>
#include <sys/eventfd.h>

using namespace sserver;
using namespace sserver::net;

namespace
{ //当前线程eventloop对象指针
  //线程局部存储
__thread EventLoop *t_loopInThisThread = 0; //初始化是空指针

const int kPollTimeMs = 10000;

int createEventfd()
{
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0)
  {
    LOG_SYSERR << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe
{
public:
  IgnoreSigPipe()
  {
    ::signal(SIGPIPE, SIG_IGN);
    // LOG_TRACE << "Ignore SIGPIPE";
  }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;
} // namespace

EventLoop *EventLoop::getEventLoopOfCurrentThread()
{
  return t_loopInThisThread;
}

EventLoop::EventLoop() //调用构造函数创建对象
    : looping_(false),
      quit_(false),
      eventHandling_(false),
      callingPendingFunctors_(false),
      iteration_(0),
      threadId_(CurrentThread::tid()),
      poller_(Poller::newDefaultPoller(this)), //构造了一个Poller实体对象，是ppoller或者epoller，通过newdefaultpoller函数来判断
      timerQueue_(new TimerQueue(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      currentActiveChannel_(NULL)
{
  LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_; //没个线程最多有一个eventloop对象
  if (t_loopInThisThread)
  { //如果当前线程已经创建了eventloop对象，终止log_fatal
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread
              << " exists in this thread " << threadId_;
  }
  else
  {
    t_loopInThisThread = this;
  }
  wakeupChannel_->setReadCallback(
      std::bind(&EventLoop::handleRead, this)); //注册wakeup回调
  // we are always reading the wakeupfd
  wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() //析构函数变量滞空
{
  LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
            << " destructs in thread " << CurrentThread::tid();
  wakeupChannel_->disableAll();
  wakeupChannel_->remove();
  ::close(wakeupFd_);
  t_loopInThisThread = NULL;
}

void EventLoop::loop() //io线程
{                      //事件循环，该函数不能跨线程调用，只能在创建该对象的线程中调用
  assert(!looping_);   //断言处于创建该线程对象中
  assertInLoopThread();
  looping_ = true;
  quit_ = false; // FIXME: what if someone calls quit() before loop() ?
  LOG_TRACE << "EventLoop " << this << " start looping";

  while (!quit_)
  {
    activeChannels_.clear();
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_); //kPolltimems是超时时间，这个超时时间默认给的10s，相当于一直没有时间10s之后返回一次
    ++iteration_;
    if (Logger::logLevel() <= Logger::TRACE)
    {
      printActiveChannels(); //日志处理
    }
    // TODO sort channel by priority
    eventHandling_ = true;
    for (ChannelList::iterator it = activeChannels_.begin();
         it != activeChannels_.end(); ++it)
    {
      currentActiveChannel_ = *it;                         //当前遍历的通道
      currentActiveChannel_->handleEvent(pollReturnTime_); //处理事件
    }
    currentActiveChannel_ = NULL; //全部处理完
    eventHandling_ = false;
    doPendingFunctors(); //让io线程也能执行一些任务，可以添加一些计算任务来让他们执行
  }

  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit() //该函数可以跨线程调用，不一定总是在io线程调用
{
  quit_ = true; //quit置为true，上面eventloop中的loop直接就退出了
  // There is a chance that loop() just executes while(!quit_) and exists,
  // then EventLoop destructs, then we are accessing an invalid object.
  // Can be fixed using mutex_ in both places.
  if (!isInLoopThread()) //如果不是io线程，先wakeup唤醒
  {
    wakeup(); //因为如果不是在io线程调用，现在需要quit的io线程很可能还是poll的阻塞状态或者是在处理事件，所以需要先weekup
              //需要一个唤醒通道，需要管道，eventfd
  }
}
// 在io线程中执行某个回调函数，该函数可以保证不用锁的情况下可以跨线程调用
void EventLoop::runInLoop(const Functor &cb) //因为有了runinloop这个函数，我们就可以添加部分任务让io线程执行，
//如果是当前io线程，就直接执行，不是当前io线程，则添加到queueinloop
{
  if (isInLoopThread())
  {
    cb(); //如果当前是io线程调用runinloop，则同步调用cb
  }
  else
  {
    queueInLoop(cb); //如果是其他线程调用runinloop，则异步的将cb添加到队列
  }
}

void EventLoop::queueInLoop(const Functor &cb) //将任务添加到队列中
{
  {
    MutexLockGuard lock(mutex_);
    pendingFunctors_.push_back(cb); //将任务添加到队列中
  }

  //调用queuelnloop的线程不是当前io线程需要唤醒
  //或者调用queuelnloop的线程是当前io线程，并且此时正在调用pending functor，需要唤醒
  //只有当前io线程的事件回调中调用queueinloop才不需要唤醒
  if (!isInLoopThread() || callingPendingFunctors_)
  {
    wakeup();
  }
}

TimerId EventLoop::runAt(const Timestamp &time, const TimerCallback &cb)//同下
{
  return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback &cb)
{
  Timestamp time(addTime(Timestamp::now(), delay));
  return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback &cb)
{
  Timestamp time(addTime(Timestamp::now(), interval));
  return timerQueue_->addTimer(cb, time, interval);
}

// FIXME: remove duplication
// 在io线程中执行某个回调函数，该函数可以跨线程调用
void EventLoop::runInLoop(Functor &&cb)
{
  if (isInLoopThread())
  {
    cb(); //如果当前是io线程调用runinloop，则同步调用cb
  }
  else
  {
    queueInLoop(std::move(cb)); //如果是其他线程调用runinloop，则异步的将cb添加到队列
  }
}

void EventLoop::queueInLoop(Functor &&cb)
{
  {
    MutexLockGuard lock(mutex_);
    pendingFunctors_.push_back(std::move(cb)); // emplace_back
  }

  if (!isInLoopThread() || callingPendingFunctors_)
  {
    wakeup();
  }
}

TimerId EventLoop::runAt(const Timestamp &time, TimerCallback &&cb)
{                                                         //在某个时间运行定时器任务
  return timerQueue_->addTimer(std::move(cb), time, 0.0); //把cb添加
}

TimerId EventLoop::runAfter(double delay, TimerCallback &&cb)
{ //在过多久之后运行定时器任务
  Timestamp time(addTime(Timestamp::now(), delay));
  return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback &&cb)
{                                                      //每过多久就运行一次定时器事件
  Timestamp time(addTime(Timestamp::now(), interval)); //间隔时间传递进去
  return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId timerId)
{
  return timerQueue_->cancel(timerId);
}

void EventLoop::updateChannel(Channel *channel)
{
  assert(channel->ownerLoop() == this); //channel是属于本对象的才可以用
  assertInLoopThread();
  poller_->updateChannel(channel); //调用了poller的update
}
//调用这个函数前确保调用了disableAll
void EventLoop::removeChannel(Channel *channel)
{
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  if (eventHandling_)
  {
    assert(currentActiveChannel_ == channel ||
           std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
  }
  poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{ //如果不在此线程调用，则直接退出
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " << CurrentThread::tid();
}

void EventLoop::wakeup() //一个线程唤醒io线程
{
  uint64_t one = 1;
  ssize_t n = sockets::write(wakeupFd_, &one, sizeof one); //写入
  if (n != sizeof one)
  {
    LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::handleRead()
{
  uint64_t one = 1;
  ssize_t n = sockets::read(wakeupFd_, &one, sizeof one); //调用read函数
  if (n != sizeof one)
  {
    LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}

void EventLoop::doPendingFunctors()
{
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
    MutexLockGuard lock(mutex_);     //保护临街区
    functors.swap(pendingFunctors_); //进行交换，避免了死锁出现
    //这样一方面减小了临界区的长度（意味着不会阻塞其他线程的queueinloop()）,
    //另一方面，也避免了死锁，（因为functor可能会在次调用queueinloop()）
  }

  for (size_t i = 0; i < functors.size(); ++i)
  {
    functors[i](); //遍历函数列表，不需要加锁了
    //由于dopendingfunctors()调用的functor可能会再次调用queueinloop(cb),这时
    //queueinloop()就必须wakeup()，否则新增的cb可能就不能及时调用了
  }
  callingPendingFunctors_ = false;
  //没有反复调用dopendingfunctor()直到pendingfunctors为空，这是有意的，否则
  //io线程可能会陷入死循环，无法处理io事件
}

void EventLoop::printActiveChannels() const
{
  for (ChannelList::const_iterator it = activeChannels_.begin();
       it != activeChannels_.end(); ++it)
  {
    const Channel *ch = *it;
    LOG_TRACE << "{" << ch->reventsToString() << "} ";
  }
}
