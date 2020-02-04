#ifndef SSERVER_COUNTDOWNLATCH_H
#define SSERVER_COUNTDOWNLATCH_H

#include "Condition.h"
#include "Mutex.h"
//也相当一个门栓类
//既可以用于所有的子线程等待主线程发起起跑
//也可以用与主线程等待子线程初始化完毕才开始工作
namespace sserver
{

class CountDownLatch
{
public:
    explicit CountDownLatch(int count);
    CountDownLatch(const CountDownLatch &) = delete; //防止拷贝构造
    CountDownLatch &operator=(const CountDownLatch &) = delete;
    void wait();

    void countDown();

    int getCount() const;

private:
    mutable MutexLock mutex_; //可变的，所以可以在const函数中改变状态
    Condition condition_;
    int count_;
};

} // namespace sserver
#endif // SSERVER_COUNTDOWNLATCH_H
