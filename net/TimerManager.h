/*
 * @Date: 2021-02-05 23:19:04
 * @LastEditors: OBKoro1
 * @LastEditTime: 2021-02-05 23:59:55
 * @FilePath: /ssxrver/net/TimerManager.h
 * @Auther: SShouxun
 * @GitHub: https://github.com/RandyLambert
 */
#ifndef SSXRVER_NET_TIMERMANAGER_H
#define SSXRVER_NET_TIMERMANAGER_H
#include <boost/noncopyable.hpp>
#include <queue>
#include <vector>
#include <memory>
#include "Timer.h"
#include "Channel.h"
namespace ssxrver::net
{

class EventLoop;
struct TimerCmp
{
    bool operator()(const std::shared_ptr<Timer> &a,const std::shared_ptr<Timer> &b){
        if(a->getExpiration() == b->getExpiration()){
            return a->getSeq() > b->getSeq();
        } else {
            return a->getExpiration() > b->getExpiration();
        }
    }
};

class TimerManager : boost::noncopyable
{
public:
    using TimerQueue = std::priority_queue<std::shared_ptr<Timer>,std::vector<std::shared_ptr<Timer>>,TimerCmp>;
    explicit TimerManager(EventLoop *loop);
    ~TimerManager();

    std::weak_ptr<Timer> addTimer(TimerCallback cb,
                  Timer::TimePoint when,
                  Timer::TimeUnit interval);
    void cancel(const std::weak_ptr<Timer>& timeId);

private:

    void addTimerInLoop(const std::shared_ptr<Timer>& timer);
    static void cancelInLoop(const std::weak_ptr<Timer>& timeId);

    void handleRead();
    std::vector<std::shared_ptr<Timer>> getExpired(const Timer::TimePoint& now);
    void reset(const std::vector<std::shared_ptr<Timer>> &expired,const Timer::TimePoint& now);

    bool insert(const std::shared_ptr<Timer>& timer);

    EventLoop *loop_;
    const int timerFd_;
    Channel timerFdChannel_;
    TimerQueue timers_;
};

}


#endif // SSXRVER_NET_TIMERMANAGER_H