/*
 * @Date: 2021-02-05 20:08:09
 * @LastEditors: OBKoro1
 * @LastEditTime: 2021-02-05 23:18:16
 * @FilePath: /ssxrver/net/Timer.h
 * @Auther: SShouxun
 * @GitHub: https://github.com/RandyLambert
 */

#ifndef SSXRVER_NET_TIMER_H
#define SSXRVER_NET_TIMER_H
#include <boost/noncopyable.hpp>
#include <chrono>
#include <utility>
#include "CallBacks.h"
namespace ssxrver::net
{
class Timer : boost::noncopyable
{
public:
    using TimeUnit = std::chrono::nanoseconds;
    using TimePoint = std::chrono::steady_clock::time_point;
    Timer(TimerCallback cb, TimePoint when, TimeUnit interval)
        : callback_(std::move(cb)),
          expiration_(when),
          interval_(interval),
          repeat_(interval_.count() > 0),
          Delete_(false),
          seq_(sTimerNum_++)
        {
        }
    [[nodiscard]] const TimePoint& getExpiration() const { return expiration_;}
    [[nodiscard]] int64_t getSeq() const {return seq_; }
    [[nodiscard]] bool isRepeat() const { return repeat_; }
    [[nodiscard]] bool isDelete() const { return Delete_;}
    void setDelete(bool on){ Delete_ = on; }
    void restart( const TimePoint& now) { expiration_ = now + interval_; }
    void run() const {callback_(); }
private:
    const TimerCallback callback_;
    TimePoint expiration_; // 距离下一次触发需要的时间
    TimeUnit interval_;   //
    const bool repeat_;
    bool Delete_;
    const int64_t seq_;
    static int64_t sTimerNum_;
};
}



#endif //SSXRVER_TIMER_H
