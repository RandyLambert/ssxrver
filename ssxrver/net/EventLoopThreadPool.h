#ifndef SSXRVER_BASE_EVENTLOOPTHREADPOOL_H
#define SSXRVER_BASE_EVENTLOOPTHREADPOOL_H
#include <vector>
#include <functional>
#include <memory>
#include "../base/noncopyable.h"
namespace ssxrver
{
namespace net
{
class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    EventLoopThreadPool(EventLoop *baseLoop);
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start();
    EventLoop *getNextLoop();

    bool started() const { return started_; }

private:
    EventLoop *baseLoop_;
    bool started_;
    int numThreads_;
    size_t next_;
    std::vector<std::shared_ptr<EventLoopThread>> threads_; //io线程列表
    std::vector<EventLoop *> loops_;
};

} // namespace net
} // namespace ssxrver
#endif
