#ifndef SSXRVER_BASE_CONDITION_H
#define SSXRVER_BASE_CONDITION_H
#include <pthread.h>
#include <errno.h>
#include "Mutex.h"
#include "noncopyable.h"
namespace ssxrver
{
class Condition : noncopyable
{
public:
    explicit Condition(MutexLock & mutex):mutex_(mutex) {
        MCHECK(pthread_cond_init(&cond_,nullptr));
    }
    ~Condition() {  MCHECK(pthread_cond_destroy(&cond_));}
    void wait()
    {
        MutexLock::UnassignGuard temp(mutex_);
        pthread_cond_wait(&cond_,mutex_.get());
    }
    void notifyOne(){MCHECK(pthread_cond_signal(&cond_));}
    void notifyAll(){MCHECK(pthread_cond_broadcast(&cond_));}
    bool waitForSeconds(int seconds)
    {
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME,&abstime);

        const int64_t kNanoSecondsPerSecond = 1000000000;
        int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);

        abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds) / kNanoSecondsPerSecond);
        abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanoseconds) % kNanoSecondsPerSecond);

        MutexLock::UnassignGuard ug(mutex_); //解锁在加锁的类
        return ETIMEDOUT == pthread_cond_timedwait(&cond_, mutex_.get(), &abstime);
    }

private:
    MutexLock &mutex_;
    pthread_cond_t cond_;
};

}
#endif
