#ifndef SSXRVER_BASE_COUNTDOWMLATCH_H
#define SSXRVER_BASE_COUNTDOWMLATCH_H
#include "Condition.h"
#include "Mutex.h"
#include "noncopyable.h"
namespace ssxrver
{
//也相当一个门栓类
//既可以用于所有的子线程等待主线程发起起跑
//也可以用与主线程等待子线程初始化完毕才开始工作
class CountDownLatch : noncopyable
{
public:
    explicit CountDownLatch(int count);
    void wait();
    void countDown();

private:
    MutexLock mutex_;
    Condition condition_;
    int count_;
};

} // namespace ssxrver
#endif
