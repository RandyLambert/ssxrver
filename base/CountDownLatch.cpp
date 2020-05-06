#include "CountDownLatch.h"
using namespace ssxrver;

CountDownLatch::CountDownLatch(int count)
    : mutex_(),
      condition_(mutex_),
      count_(count)
{
}

void CountDownLatch::wait()
{
    MutexLockGuard lock(mutex_);
    while (count_)
    {
        condition_.wait();
    }
}

void CountDownLatch::countDown()
{
    MutexLockGuard lock(mutex_);
    if (--count_ == 0)
    {
        condition_.notifyAll();
    }
}
