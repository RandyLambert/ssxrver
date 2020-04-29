#include <functional>
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"

using namespace ssxrver;
using namespace ssxrver::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop)
    : baseLoop_(baseLoop),
      started_(false),
      numThreads_(0),
      next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool() = default;

void EventLoopThreadPool::start()
{
    assert(!started_);
    baseLoop_->assertInLoopThread();
    started_ = true;

    for (int i = 0; i < numThreads_; ++i)
    {
        std::shared_ptr<EventLoopThread> t(new EventLoopThread());
        threads_.push_back(t);
        loops_.push_back(t->startLoop()); //启动eventloopthreads线程，在进入事件循环之前，会调用cb
    }
}

EventLoop *EventLoopThreadPool::getNextLoop()
{
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop *loop = baseLoop_;
    //如果loops为空，则loop指向baseloop_
    //如果不为空，按照round-robin轮叫的调度方式选择一个eventloop

    if (!loops_.empty())
    {
        //round-robin
        loop = loops_[next_];
        next_ = (next_ + 1) % numThreads_;
    }
    return loop;
}
