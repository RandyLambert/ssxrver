#ifndef SSXRVER_NET_EVENTLOOPTHREAD_H
#define SSXRVER_NET_EVENTLOOPTHREAD_H
#include <boost/noncopyable.hpp>
#include <mutex>
#include <condition_variable>
#include "../base/Thread.h"
namespace ssxrver
{
namespace net
{
class EventLoop;

class EventLoopThread : boost::noncopyable
{
public:
    typedef std::function<void(EventLoop *)> ThreadInitCallback;

    explicit EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback());
    ~EventLoopThread();
    EventLoop *startLoop();

private:
    void threadFunc(); //传到线程里面的函数

    EventLoop *loop_; //loop_指针指向一个eventloop对象
    bool exiting_;    //是否退出
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};
} // namespace net
} // namespace ssxrver

#endif
