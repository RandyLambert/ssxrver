#include "ThreadPool.h"
#include "Exception.h"

#include <assert.h>
#include <stdio.h>
#include <functional>
using namespace sserver;

ThreadPool::ThreadPool(const string& name)
    : mutex_(),
    notEmpty_(mutex_),
    notFull_(mutex_),
    name_(name),
    maxQueueSize_(0),
    running_(false)
{
}

ThreadPool::~ThreadPool()//析构函数，如果线程池在运行状态，就停止线程池
{
    if (running_)
    {
        stop();
    }
}

void ThreadPool::start(int numThreads)//启动固定的线程
{
    assert(threads_.empty());//断言线程池为空
    running_ = true;//判断为true
    threads_.reserve(numThreads);//预先申请线程数的内存
    for (int i = 0; i < numThreads; ++i)//创建足够的线程
    {
        char id[32];
        snprintf(id, sizeof id, "%d", i+1);//
        threads_.push_back(new sserver::Thread( //创建线程加入线程vector，线程名叫做线程池的名字+id
                                               std::bind(&ThreadPool::runInThread, this), name_+id));
        threads_[i].start();//启动线程
    }
    if (numThreads == 0 && threadInitCallback_)//??
    {
        threadInitCallback_();
    }
}

void ThreadPool::stop()
{
    {
        MutexLockGuard lock(mutex_);//获取锁的保护
        running_ = false;
        notEmpty_.notifyAll();//通知所有等待线程，但是因为running变为了false，所以线程结束
    }
    for_each(threads_.begin(),
             threads_.end(),//对所有线程调用join，等待所有线程
             bind(&sserver::Thread::join, std::placeholders::_1));
}

void ThreadPool::run(const Task& task)
{
    if (threads_.empty())//如果发现线程队列为空，就直接执行任务，没有空闲线程
    {
        task();
    }
    else
    {
        MutexLockGuard lock(mutex_);//用锁保护
        while (isFull())//????
        {
            notFull_.wait();
        }
        assert(!isFull());

        queue_.push_back(task);//将任务加入队列
        notEmpty_.notify();//通知队列中有任务了
    }
}

void ThreadPool::run(Task&& task)//同上
{
    if (threads_.empty())
    {
        task();
    }
    else
    {
        MutexLockGuard lock(mutex_);
        while (isFull())
        {
            notFull_.wait();
        }
        assert(!isFull());

        queue_.push_back(std::move(task));
        notEmpty_.notify();
    }
}

ThreadPool::Task ThreadPool::take()
{
    //任务队列需要保护
    MutexLockGuard lock(mutex_);
    // always use a while-loop, due to spurious wakeup
    while (queue_.empty() && running_) //等待的条件有两种，要么是有任务到来，要摸就是线程池结束
    {
        notEmpty_.wait();
    }
    Task task;
    if (!queue_.empty()) //一旦有任务到来，任务队列不为空
    {
        task = queue_.front();//弹出任务
        queue_.pop_front();
        if (maxQueueSize_ > 0) //？？
        {
            notFull_.notify();
        }
    }
    return task;//取出任务
}

bool ThreadPool::isFull() const
{
    mutex_.assertLocked();
    return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

void ThreadPool::runInThread()
{
    try
    {
        if (threadInitCallback_)
        {
            threadInitCallback_();
        }
        while (running_)//如果running为true，则在这个循环里执行线程
        {
            Task task(take());//获取任务
            if (task)
            {
                task();
            }
        }
    }
    catch (const Exception& ex) //异常捕捉
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
        abort();
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    }
    catch (...)
    {
        fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
        throw; // rethrow
    }
}
