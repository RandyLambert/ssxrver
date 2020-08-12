#ifndef SSXRVER_NET_EVENTLOOP_H
#define SSXRVER_NET_EVENTLOOP_H
#include <vector>
#include <memory>
#include <functional>
#include <atomic>
#include <boost/noncopyable.hpp>
#include <mutex>
#include "../base/CurrentThread.h"
#include "../base/Thread.h"
#include "MySQLsOps.h"
namespace ssxrver
{
namespace net
{
class Channel;
class Epoller;

class EventLoop : boost::noncopyable
{
public:
    using Functor = std::function<void()>;
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
        if (!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
    bool eventHandling() const { return eventHandling_; }
//    MySQLsOps *getMySQL() { return &mysql_; }

private:
    void abortNotInLoopThread();
    void handleRead();
    void doPendingFunctors(); //执行转交给I\O的任务

    typedef std::vector<Channel *> ChannelList; //事件分发器列表
    bool looping_;
    std::atomic<bool> quit_;
    bool eventHandling_;
    bool callingPendingFuctors_;
    const pid_t threadId_;

    std::unique_ptr<Epoller> Epoller_;

    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;

    mutable std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;
//    MySQLsOps mysql_;
};
} // namespace net
} // namespace ssxrver
#endif
