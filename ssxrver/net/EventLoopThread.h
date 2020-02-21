#ifndef SSXRVER_NET_EVENTLOOPTHREAD_H
#define SSXRVER_NET_EVENTLOOPTHREAD_H
#include "../base/noncopyable.h"
#include "../base/Thread.h"
#include "../base/Mutex.h"
namespace ssxrver
{
namespace net
{
class EventLoop;

class EventLoopThread : noncopyable
{
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop *startLoop();

private:
    void threadFunc(); //传到线程里面的函数

    EventLoop *loop_; //loop_指针指向一个eventloop对象
    bool exiting_;   //是否退出
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
};
}
}

#endif
