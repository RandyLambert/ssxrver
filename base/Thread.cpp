#include <memory>
#include <cerrno>
#include <cstdio>
#include <unistd.h>
#include <sys/prctl.h>
#include <cassert>
#include "CurrentThread.h"
#include "Thread.h"
#include "Exception.h"
#include "Logging.h"

namespace ssxrver
{

    namespace CurrentThread
    {
        thread_local int t_cachedTid = 0;
        thread_local string t_tidString;
        thread_local string t_threadName;
    } // namespace CurrentThread

    namespace detail
    {

        using ThreadFunc = ssxrver::Thread::ThreadFunc;

        void runInThread(ThreadFunc func,const string& name,pid_t *tid,CountDownLatch *latch) //真正创建线程后,穿进去的函数会直接调用他
        {
            *tid = ssxrver::CurrentThread::tid();
            latch->countDown();

            ssxrver::CurrentThread::t_threadName = name.empty() ? "ssxrverThread" : name;
            ::prctl(PR_SET_NAME, ssxrver::CurrentThread::t_threadName.c_str());

            try
            {
                func();                                           //传入函数
                ssxrver::CurrentThread::t_threadName = "finished"; //线程状态
            }
            catch (const Exception &ex) //异常捕捉，先在自己写的函数，在是函数库，最后是不得已的捕捉
            {
                ssxrver::CurrentThread::t_threadName = "crashed";
                fprintf(stderr, "exception caught in Thread %s\n", name.c_str());
                fprintf(stderr, "reason: %s\n", ex.what());
                fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
                abort();
            }
            catch (const std::exception &ex)
            {
                ssxrver::CurrentThread::t_threadName = "crashed";
                fprintf(stderr, "exception caught in Thread %s\n", name.c_str());
                fprintf(stderr, "reason: %s\n", ex.what());
                abort();
            }
            catch (...)
            {
                ssxrver::CurrentThread::t_threadName = "crashed";
                fprintf(stderr, "unknown exception caught in Thread %s\n", name.c_str());
                throw; // rethrow
            }
        }

    } // namespace detail

} // namespace ssxrver
using namespace ssxrver;

bool CurrentThread::isMainThread()
{
    return tid() == getpid(); //判断这个线程的tid是不是主线程的pid，如果是，就说明是主线程
}

Thread::Thread(ThreadFunc func,string n)
        : started_(false),
          tid_(0),
          func_(std::move(func)),
          name_(std::move(n)),
          latch_(1)
{
}

Thread::~Thread()
{
    if (started_ && !thread_->joinable())
        thread_->join();
}

void Thread::start()
{
    assert(!started_);
    started_ = true;
    thread_ = std::make_unique<std::thread>(std::thread(detail::runInThread,func_,name_,&tid_,&latch_)); //作为线程参数传进去
    if (thread_== nullptr)
    {
        started_ = false;
        LOG_SYSFATAL << "fail in pthread_create";
    }
    else
    {
        latch_.wait();
        assert(tid_ > 0);
    }
}