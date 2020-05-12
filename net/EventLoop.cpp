#include <sys/eventfd.h>
#include <signal.h>
#include "EventLoop.h"
#include "../base/Logging.h"
#include "../base/Mutex.h"
#include "Channel.h"
#include "Epoller.h"
#include "SocketOps.h"

using namespace ssxrver;
using namespace ssxrver::net;
namespace
{
__thread EventLoop *t_loopInThisThread = nullptr;
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_SYSFATAL << "fail in eventfd";
        abort();
    }
    return evtfd;
}
class SigPipeinit
{
public:
    SigPipeinit()
    {
        ::signal(SIGPIPE, SIG_IGN);
        ssxrver::CurrentThread::t_threadName = "main";
        CurrentThread::tid();
    }
};
SigPipeinit initObj;
} // namespace

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      eventHandling_(false),
      callingPendingFuctors_(false),
      threadId_(CurrentThread::tid()),
      Epoller_(new Epoller(this)),
      wakeupFd_(createEventfd()), //创建一个eventfd
      wakeupChannel_(new Channel(this, wakeupFd_))
//   mysql_()
{
    LOG_DEBUG << "EventLoop created " << this << "in thread " << threadId_; //每个线程最多一个eventLoop
    if (t_loopInThisThread)
    {
        LOG_FATAL << "Another EventLoop" << t_loopInThisThread
                  << "exists in this thread " << threadId_;
    }
    else
        t_loopInThisThread = this; //否则将刚出案件的线程付给指针
    wakeupChannel_->setReadCallback(
        std::bind(&EventLoop::handleRead, this)); //注册wakeup的回调函数
    /* Epoller_->name_ = "io循环"; */
    wakeupChannel_->enableReading(); //在这里收纳到epoller管理便于唤醒
}
/* void EventLoop::setname(string name) { Epoller_->name_ = name; } */
EventLoop::~EventLoop()
{
    LOG_DEBUG << "EventLoop " << this << "of thread " << threadId_
              << " destructs in thread " << CurrentThread::tid();
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    while (!quit_)
    {
        activeChannels_.clear();
        Epoller_->poll(&activeChannels_);
        eventHandling_ = true;
        for (Channel *channel : activeChannels_)
        {
            // LOG_INFO << "handleEvent";
            channel->handleEvent();
            // LOG_INFO << "handleEventafter";
        }
        eventHandling_ = false;
        doPendingFunctors(); //让io线程也可以执行一些小的任务
    }

    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    if (!isInLoopThread())
        wakeup();
}

void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
        cb();
    else
        queueInLoop(std::move(cb));
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }

    if (!isInLoopThread() || callingPendingFuctors_)
        wakeup();
}

size_t EventLoop::queueSize() const
{
    MutexLockGuard lock(mutex_);
    return pendingFunctors_.size();
}

void EventLoop::updateChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    Epoller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    Epoller_->removeChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << threadId_
              << ", current thread id = " << CurrentThread::tid();
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    /* LOG_INFO << "WeakUp 函数"; */
    if (::write(wakeupFd_, &one, sizeof(one)) != sizeof(one))
        LOG_ERROR << "eventloop::wakeup() writes "
                  << "another bytes instead of 8";
}

void EventLoop::handleRead() //处理wakeup
{
    uint64_t one = 1;
    if (::read(wakeupFd_, &one, sizeof(one)) != sizeof(one))
        LOG_ERROR << "eventloop::wakeup() read "
                  << "another bytes instead of 8";
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFuctors_ = true;

    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor &functor : functors)
        functor();

    callingPendingFuctors_ = false;
}
