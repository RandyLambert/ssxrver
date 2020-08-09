#include "CountDownLatch.h"
using namespace ssxrver;

CountDownLatch::CountDownLatch(int count)
    : mutex_(),
      condition_(),
      count_(count)
{
}

void CountDownLatch::wait()
{
    std::unique_lock <std::mutex> lock(mutex_);
    while (count_)
    {
        condition_.wait(lock);
    }
}

void CountDownLatch::countDown()
{
    std::unique_lock <std::mutex> lock(mutex_);
    if (--count_ == 0)
    {
        condition_.notify_all();
    }
}
