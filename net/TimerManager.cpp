/*
 * @Date: 2021-02-05 23:19:29
 * @LastEditors: OBKoro1
 * @LastEditTime: 2021-02-06 00:09:19
 * @FilePath: /ssxrver/net/TimerManager.cpp
 * @Auther: SShouxun
 * @GitHub: https://github.com/RandyLambert
 */
#include "TimerManager.h"
#include "../base/Logging.h"
#include "Timer.h"
#include "EventLoop.h"
#include <sys/timerfd.h>
namespace ssxrver::net::detail
{

int createTimerFd()
{
    int timerFd = ::timerfd_create(CLOCK_MONOTONIC,
                                   TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerFd < 0)
    {
        LOG_SYSFATAL << "Failed in timerfd_create";
    }
    return timerFd;
}

void readTimerFd(int timerFd)
{
    uint64_t howMany;
    ssize_t n = ::read(timerFd, &howMany, sizeof howMany);
    LOG_DEBUG << "TimerQueue::handleRead() " << howMany;
    if (n != sizeof(howMany))
    {
        LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
    }
}
 //重置定时器的超时时间
 void resetTimerFd(int timerFd, Timer::TimePoint expiration)
 {
     // wake up loop by timerfd_settime()
     struct itimerspec newValue{};
     Timer::TimeUnit nsec = expiration - std::chrono::steady_clock::now();
     newValue.it_value.tv_sec  = std::chrono::duration_cast<std::chrono::duration<int64_t>>(nsec).count();
     newValue.it_value.tv_nsec = nsec.count() % 1000000000;
     int ret = ::timerfd_settime(timerFd, 0, &newValue, nullptr);
     if (ret)
     {
         LOG_SYSERR << "timerfd_settime()";
     }
 }
} // namespace sserver::net::detail

using namespace ssxrver;
using namespace ssxrver::net;
using namespace ssxrver::net::detail;
TimerManager::TimerManager(EventLoop *loop)
    : loop_(loop),
      timerFd_(createTimerFd()),
      timerFdChannel_(loop_,timerFd_),
      timers_()
{
    timerFdChannel_.setReadCallback([this]{handleRead();});
    timerFdChannel_.enableEvents(kReadEventLT);
}

TimerManager::~TimerManager()
{
    timerFdChannel_.disableAll();
    timerFdChannel_.remove();
    ::close(timerFd_);
}
// ????
std::weak_ptr<Timer> TimerManager::addTimer(TimerCallback cb, Timer::TimePoint when, Timer::TimeUnit interval)
{
    std::shared_ptr<Timer> timer = std::make_shared<Timer>(std::move(cb),when,interval);
            loop_->runInLoop( [this,timer]{addTimerInLoop(timer); });
    if(timer == nullptr){
        LOG_WARN<<"timer == nullptr";
    }
    return timer;
}

void TimerManager::cancel(const std::weak_ptr<Timer>& timeId)
{
    loop_->runInLoop([timeId]{ cancelInLoop(timeId); });
}

void TimerManager::handleRead() {
    loop_->assertInLoopThread();
    readTimerFd(timerFd_);
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    std::vector<std::shared_ptr<Timer>> expired = getExpired(now);
    for(const auto& key : expired)
    {
        if(!key->isDelete())
            key->run();
    }
    reset(expired,now);
}

void TimerManager::addTimerInLoop(const std::shared_ptr<Timer>& timer) {
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);
    if(earliestChanged) {
        resetTimerFd(timerFd_,timer->getExpiration());
    }
}

bool TimerManager::insert(const std::shared_ptr<Timer>& timer) {
    loop_->assertInLoopThread();
    bool earliestChanged = false;
    if(!timers_.empty()) {
        if(timers_.top()->getExpiration() > timer->getExpiration())
        {
            earliestChanged = true;
        }
        timers_.push(timer);
    } else {
        timers_.push(timer);
        earliestChanged = true;
    }
    return earliestChanged;
}

std::vector<std::shared_ptr<Timer>> TimerManager::getExpired(const Timer::TimePoint& now) {
    std::vector<std::shared_ptr<Timer>> expired;
    while(!timers_.empty() && timers_.top()->getExpiration() <= now) {
        if(!timers_.top()->isDelete())
            expired.emplace_back(timers_.top());
        timers_.pop();
    }
    return expired;
}

void TimerManager::reset(const std::vector<std::shared_ptr<Timer>> &expired, const Timer::TimePoint& now) {

    for(const auto& key : expired) {
        if(key->isRepeat() && !key->isDelete()) {
            key->restart(now);
            insert(key);
        }
    }
    while(!timers_.empty()) {
        if(!timers_.top()->isDelete()){
            resetTimerFd(timerFd_,timers_.top()->getExpiration());
            break;
        } else {
            timers_.pop();
        }
    }
}

void TimerManager::cancelInLoop(const std::weak_ptr<Timer>& timeId) {
    if (!timeId.expired()) {
        auto guard = timeId.lock(); //weak_ptr的使用
        guard->setDelete(true);
    }
}
