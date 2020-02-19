#ifndef SSXRVER_BASE_MUTEX_H
#define SSXRVER_BASE_MUTEX_H
#include <assert.h>
#include <pthread.h>
#include "noncopyable.h"
#include "CurrentThread.h"
#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum; })

namespace ssxrver
{
class MutexLock : noncopyable
{
public:
    MutexLock()
    {
       MCHECK(pthread_mutex_init(&mutex_,nullptr));
    }
    ~MutexLock()
    {
        MCHECK(pthread_mutex_destroy(&mutex_));
    }

    bool isLockByThisThread() const{    return CurrentThread::tid() == holder_;}
    void assertLocked() const{  assert(isLockByThisThread());}

    void lock()
    {
        pthread_mutex_lock(&mutex_);
        holder_ = CurrentThread::tid();
    }

    void unlock(){
        holder_ = 0;
        pthread_mutex_unlock(&mutex_);
    }
    pthread_mutex_t *get(){ return &mutex_; }

private:
    friend class Condition;
    class UnassignGuard : noncopyable
    {
    public:
        explicit UnassignGuard(MutexLock &owner): owner_(owner){    owner_.holder_ = 0;}
        ~UnassignGuard(){    owner_.holder_ = CurrentThread::tid();}

    private:
        MutexLock & owner_;
    };
    pthread_mutex_t mutex_;
    pid_t holder_;
};

class MutexLockGuard : noncopyable {
 public:
  explicit MutexLockGuard(MutexLock &_mutex) : mutex(_mutex) { mutex.lock(); }
  ~MutexLockGuard() { mutex.unlock(); }

 private:
  MutexLock &mutex;
};

}
#endif
