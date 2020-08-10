#include <functional>
#include <cassert>
#include "EventLoopThread.h"
#include "EventLoop.h"
using namespace ssxrver;
using namespace ssxrver::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb)
    : loop_(nullptr),
      exiting_(false),
      thread_(std::bind(&EventLoopThread::threadFunc, this)),
      mutex_(),
      cond_(),
      callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->quit();
    }
}

EventLoop *EventLoopThread::startLoop()
{

    assert(!thread_.started());
    thread_.start();
    {
        std::unique_lock <std::mutex> lock(mutex_);
        while (loop_ == nullptr)
        {
            cond_.wait(lock);
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
        std::unique_lock <std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();
    std::unique_lock <std::mutex> lock(mutex_);
    loop_ = nullptr;
}
