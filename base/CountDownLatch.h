#ifndef SSXRVER_BASE_COUNTDOWMLATCH_H
#define SSXRVER_BASE_COUNTDOWMLATCH_H
#include <mutex>
#include <condition_variable>
#include <boost/noncopyable.hpp>
namespace ssxrver
{
//也相当一个门栓类
//既可以用于所有的子线程等待主线程发起起跑
//也可以用与主线程等待子线程初始化完毕才开始工作
class CountDownLatch : boost::noncopyable
{
public:
    explicit CountDownLatch(int count);
    void wait();
    void countDown();

private:
    std::mutex mutex_;
    std::condition_variable condition_;
    int count_;
};

} // namespace ssxrver
#endif
