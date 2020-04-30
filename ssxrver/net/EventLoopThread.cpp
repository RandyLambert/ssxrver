#include <functional>
#include "EventLoopThread.h"
#include "EventLoop.h"
#include "../base/Logging.h"
using namespace ssxrver;
using namespace ssxrver::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb)
    : loop_(nullptr),
      exiting_(false),
      thread_(std::bind(&EventLoopThread::threadFunc, this)),
      mutex_(),
      cond_(mutex_),
      callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::startLoop()
{

    assert(!thread_.startorNot());
    thread_.start();
    {
        MutexLockGuard lock(mutex_);
        while (loop_ == nullptr)
        {
            cond_.wait();
        }
    }
    return loop_;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    if (callback_)
    {
        callback_(&loop);
    }

    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notifyOne();
    }

    loop.loop();
    loop_ = nullptr;
}
