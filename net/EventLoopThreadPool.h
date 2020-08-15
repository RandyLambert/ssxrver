#ifndef SSXRVER_NET_EVENTLOOPTHREADPOOL_H
#define SSXRVER_NET_EVENTLOOPTHREADPOOL_H
#include <vector>
#include <functional>
#include <memory>
#include <boost/noncopyable.hpp>
namespace ssxrver::net
{
class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : boost::noncopyable
{
public:
    typedef std::function<void(EventLoop *)> ThreadInitCallback;
    explicit EventLoopThreadPool(EventLoop *baseLoop);
    ~EventLoopThreadPool();
    void setThreadNum(size_t numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback &cb = ThreadInitCallback());
    EventLoop *getNextLoop();

    [[nodiscard]] bool started() const { return started_; }

private:
    EventLoop *baseLoop_;
    bool started_;
    size_t numThreads_;
    size_t next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_; //io线程列表
    std::vector<EventLoop *> loops_;
};

} // namespace ssxrver::net
#endif
