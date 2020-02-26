#ifndef SSXRVER_NET_EVENTLOOP_H
#define SSXRVER_NET_EVENTLOOP_H
#include <vector>
#include <any>
#include <memory>
#include <functional>
#include <atomic>
#include "../base/noncopyable.h"
#include "../base/CurrentThread.h"
#include "../base/Mutex.h"
#include "../base/Thread.h"
#include "../base/Logging.h"
namespace ssxrver
{
namespace net
{
class Channel;
class Epoller;

class EventLoop : noncopyable
{
public:
    typedef std::function<void()> Functor;
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();

    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);
    size_t queueSize() const;

    void wakeup();
    void updateChannel(Channel *channel); //在poller中添加或者更新通道
    void removeChannel(Channel *channel); //在poller中移除通道

    void assertInLoopThread()
    {
        if(!isInLoopThread())
        {
             LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << threadId_
              << ", current thread id = " << CurrentThread::tid();
        }
    }

    bool isInLoopThread() const {return threadId_ == CurrentThread::tid();}
    bool eventHandling() const {return eventHandling_;}

private:

    void handleRead();
    void doPendingFunctors(); //执行转交给I\O的任务
    
    typedef std::vector<Channel *> ChannelList; //时间分发器列表
    bool looping_;
    std::atomic<bool> quit_;
    bool eventHandling_;
    bool callingPendingFuctors_;
    const pid_t threadId_;
    
    std::unique_ptr<Epoller> Epoller_;

    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;

    mutable MutexLock mutex_;
    std::vector <Functor> pendingFunctors_;
};
}
}
#endif
