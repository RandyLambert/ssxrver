#include <memory>
#include <cerrno>
#include <cstdio>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <assert.h>

#include "CurrentThread.h"
#include "Thread.h"
#include "Exception.h"
#include "Logging.h"

namespace ssxrver
{
namespace CurrentThread
{
__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char *t_threadName = "noname";
}    

namespace detail
{

struct ThreadData //把整个线程数据传进去
{
    typedef ssxrver::Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;
    string name_;
    pid_t *tid_;
    CountDownLatch *latch_;

    ThreadData(ThreadFunc func,
               const string &name,
               pid_t *tid,
               CountDownLatch *latch)
        : func_(std::move(func)),
        name_(name),
        tid_(tid),
        latch_(latch)
    {
    }

    void runInThread() //真正创建线程后,穿进去的函数会直接调用他
    {
        *tid_ = ssxrver::CurrentThread::tid();
        tid_ = NULL;
        latch_->countDown();
        latch_ = NULL;

        ssxrver::CurrentThread::t_threadName = name_.empty() ? "ssxrverThread" : name_.c_str();
        ::prctl(PR_SET_NAME, ssxrver::CurrentThread::t_threadName);

        try
        {
            func_();//传入函数
            ssxrver::CurrentThread::t_threadName = "finished";//线程状态
        }
        catch (const Exception &ex) //异常捕捉，先在自己写的函数，在是函数库，最后是不得已的捕捉
        {
            ssxrver::CurrentThread::t_threadName = "crashed";
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
            abort();
        }
        catch (const std::exception &ex)
        {
            ssxrver::CurrentThread::t_threadName = "crashed";
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            abort();
        }
        catch (...)
        {
            ssxrver::CurrentThread::t_threadName = "crashed";
            fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
            throw; // rethrow
        }
    }
};

void *startThread(void *obj)//void *func(void *obj)
{
    ThreadData *data = static_cast<ThreadData *>(obj);
    data->runInThread();
    return NULL;
}

}

}
using namespace ssxrver;

bool CurrentThread::isMainThread()
{
    return tid() == getpid(); //判断这个线程的tid是不是主线程的pid，如果是，就说明是主线程
}

Thread::Thread(ThreadFunc func,const string n)
    : started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    func_(std::move(func)),
    name_(std::move(n)),
    latch_(1)
{
    setDdfaultName();
}

Thread::~Thread() 
{
    if (started_ && !joined_)
        pthread_detach(pthreadId_);
}

std::atomic<int> Thread::numCreated_;

void Thread::setDdfaultName()
{
    int num = ++numCreated_; //原子操作，自增一个线程
    if (name_.empty())                       //如果这个线程没有被命名，则默认给线程的名字叫Thread
    {
        char buf[32];
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}

void Thread::start()
{
    assert(!started_);
    started_ = true;
    detail::ThreadData *data = new detail::ThreadData(func_,name_,&tid_, &latch_); //作为线程参数穿进去
    if(pthread_create(&pthreadId_,NULL,&detail::startThread,data))
    {
        started_ = false;
        delete data; 
        LOG_SYSFATAL << "fail in pthread_create";
    }
    else
    {
        latch_.wait();
        assert(tid_ > 0);
    }
}

int Thread::join()
{
    assert(started_); //断定线程已经打开
    assert(!joined_); //要在线程打开的并且还没有join的时候join
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}
