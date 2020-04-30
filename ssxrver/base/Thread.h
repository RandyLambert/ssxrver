#ifndef SSXRVER_BASE_THREAD_H
#define SSXRVER_BASE_THREAD_H
#include <pthread.h>
#include <string>
#include <functional>
#include <atomic>
#include "CountDownLatch.h"
#include "noncopyable.h"
namespace ssxrver
{
using std::string;

class Thread : noncopyable
{
public:
    typedef std::function<void()> ThreadFunc;
    explicit Thread(ThreadFunc, const string name = string());
    ~Thread();

    void start(); //初始化
    int join();

    bool startorNot() const { return started_; }
    pid_t tid() const { return tid_; }
    const string &name() { return name_; }
    static int numCreated() { return numCreated_; }

private:
    void setDdfaultName();

    bool started_;
    bool joined_;
    pthread_t pthreadId_; //线程id，下面tid不同，可以重复，值在本进程内有用
    pid_t tid_;           //该线程在计算机中的唯一标识
    ThreadFunc func_;     //函数接口
    string name_;
    CountDownLatch latch_;

    static std::atomic<int> numCreated_;
};

} // namespace ssxrver

#endif
