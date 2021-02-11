#include <cassert>
#include <utility>
#include "EventLoopThread.h"
#include "EventLoop.h"
using namespace ssxrver;
using namespace ssxrver::net;

EventLoopThread::EventLoopThread(ThreadInitCallback cb)
    : loop_(nullptr),
      exiting_(false),
      thread_([this] { threadFunc(); }),
      mutex_(),
      cond_(),
      callback_(std::move(cb))
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
        cond_.wait(lock,[this]{return loop_ != nullptr;});
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
    std::scoped_lock <std::mutex> lock(mutex_);
    loop_ = nullptr;
}
