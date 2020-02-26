#include <functional>
#include "EventLoopThread.h"
#include "EventLoop.h"

using namespace ssxrver;
using namespace ssxrver::net;

EventLoopThread::EventLoopThread()
    : loop_(nullptr),
    exiting_(false),
    thread_(std::bind(&EventLoopThread::threadFunc,this)),
    mutex_(),
    cond_(mutex_)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop * EventLoopThread::startLoop()
{
    assert(!thread_.startorNot());
    {
        MutexLockGuard lock(mutex_);
        while(loop_ == nullptr)
        {
            cond_.wait();
        }
    }
    return loop_;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop; 
        cond_.notifyOne();
    }

    loop.loop();
    loop_ = nullptr;
}
