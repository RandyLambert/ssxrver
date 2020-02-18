#ifndef SSXRVER_BASE_MUTEX_H
#define SSXRVER_BASE_MUTEX_H
#include <assert.h>
#include <pthread.h>
#include "nonecopyable.h"
namespace ssxrver
{

class Mutex : nonecopyable
{
public:
    Mutex()
    {
        pthread_mutex_init(&mutex_,nullptr);
    }
    ~Mutex()
    {
        assert(holder_==0);
        pthread_mutex_destroy(&mutex_);
    }

    void lock()
    {
        holder_ = 

    }

    void unlock(){
        holder_ = 0;
    }


private:
    pthread_mutex_t mutex_;
    pid_t holder_;
};

class MutexLockGuard : noncopyable {
 public:
  explicit MutexLockGuard(Mutex &_mutex) : mutex(_mutex) { mutex.lock(); }
  ~MutexLockGuard() { mutex.unlock(); }

 private:
  MutexLock &mutex;
};

}


#endif
