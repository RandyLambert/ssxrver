#include <sys/eventfd.h>
#include <csignal>
#include "EventLoop.h"
#include "Channel.h"
#include "Epoll.h"
#include "TimerManager.h"
using namespace ssxrver;
using namespace ssxrver::net;
namespace
{
thread_local EventLoop *t_loopInThisThread = nullptr;
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_SYSFATAL << "fail in eventfd";
    }
    return evtfd;
}
class SigPipeinit
{
public:
    SigPipeinit()
    {
        ::signal(SIGPIPE, SIG_IGN);
        CurrentThread::tid();
    }
};
SigPipeinit initObj;
} // namespace

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      eventHandling_(false),
      callingFunctors_(false),
      threadId_(CurrentThread::tid()),
      epoll_(std::make_unique<Epoll>(this)),
      timerManger_(std::make_unique<TimerManager>(this)),
      wakeupFd_(createEventfd()), //创建一个eventFd
      wakeupChannel_(std::make_unique<Channel>(this, wakeupFd_))
{
    if (t_loopInThisThread)
    {
        LOG_FATAL << "Another EventLoop" << t_loopInThisThread
                  << "exists in this thread " << threadId_;
    }
    else
        t_loopInThisThread = this; //否则将刚出案件的线程付给指针
    wakeupChannel_->setReadCallback(
            [this] { handleRead(); }); //注册wakeup的回调函数
    wakeupChannel_->enableEvents(kReadEventLT); //在这里收纳到epoller管理便于唤醒
}

EventLoop::~EventLoop()
{
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
        epoll_->poll(&activeChannels_);
        eventHandling_ = true;
        for (Channel *channel : activeChannels_)
        {
            channel->handleEvent();
        }
        eventHandling_ = false;
        runFunctors(); //让io线程也可以执行一些小的任务
    }

    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    if (!isInLoopThread())
        wakeup();
}

void EventLoop::runInLoop(const Functor& cb)
{
    if (isInLoopThread())
        cb();
    else
        queueInLoop(cb);
}

void EventLoop::queueInLoop(const Functor& cb)
{
    {
        std::scoped_lock<std::mutex> guard(mutex_);
        functors_.emplace_back(cb);
    }

    if (!isInLoopThread() || callingFunctors_)
        wakeup();
}

void EventLoop::updateChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    epoll_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    epoll_->removeChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << threadId_
              << ", current thread id = " << CurrentThread::tid();
}

void EventLoop::wakeup() const
{
    uint64_t one = 1;
    if (::write(wakeupFd_, &one, sizeof(one)) != sizeof(one))
        LOG_ERROR << "eventLoop::wakeup() writes "
                  << "another bytes instead of 8";
}

void EventLoop::handleRead() const //处理wakeup
{
    uint64_t one = 1;
    if (::read(wakeupFd_, &one, sizeof(one)) != sizeof(one))
        LOG_ERROR << "eventloop::wakeup() read "
                  << "another bytes instead of 8";
}

void EventLoop::runFunctors()
{
    std::vector<Functor> functors;
    callingFunctors_ = true;

    {
        std::scoped_lock<std::mutex> guard(mutex_);
        functors.swap(functors_);
    }

    for (const Functor &functor : functors)
        functor();

    callingFunctors_ = false;
}

void EventLoop::createConnection(int sockFd, const ConnectionCallback &connectCallback,
                                 const MessageCallback &messageCallback, const WriteCompleteCallback &writeCompleteCallback) {

    epoll_->createConnection(sockFd, connectCallback, messageCallback, writeCompleteCallback);

}

std::weak_ptr<Timer> EventLoop::runAt(const Timer::TimePoint& time, TimerCallback cb)
{
    return timerManger_->addTimer(std::move(cb),time,Timer::TimeUnit());
}

std::weak_ptr<Timer> EventLoop::runEvery(const Timer::TimeUnit &interval, TimerCallback cb)
{
    Timer::TimePoint time = std::chrono::steady_clock::now() + interval;
    return timerManger_->addTimer(std::move(cb),time,interval);
}

std::weak_ptr<Timer> EventLoop::runAfter(const Timer::TimeUnit &delay, TimerCallback cb) {
    Timer::TimePoint time = std::chrono::steady_clock::now() + delay;
    return timerManger_->addTimer(std::move(cb),time,Timer::TimeUnit());
}