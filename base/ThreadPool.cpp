//
// Created by randylambert on 2020/11/6.
//

#include "ThreadPool.h"
using namespace ssxrver;
using namespace ssxrver::base;
ThreadPool::ThreadPool()
    : threadInitCallback_(),
      running_(false),
      taskQueue_()
{
}

ThreadPool::~ThreadPool() {
    if(running_){
        stop();
    }
}

void ThreadPool::stop() {
    running_ = false;
    for(unsigned i = 0;i < threads_.size();i++)
        taskQueue_.push([]{});
}

void ThreadPool::start(unsigned numThreads) {
    running_ = true;                     //设定为true,作为线程启动标志
    threads_.reserve(static_cast<unsigned long>(numThreads));        //预先申请线程数的内存
    for (unsigned i = 0; i < numThreads; ++i) //创建足够的线程
    {
        threads_.emplace_back(std::make_unique<Thread>([this]{ runInThread(); }));             //绑定runInThread为线程运行函数
        threads_[i]->start();                                         //启动线程
    }
    if (numThreads == 0 && threadInitCallback_) //如果线程池为空，且有回调函数，则调用回调函数。这时相当与只有一个主线程
    {
        threadInitCallback_();
    }
}

void ThreadPool::runInThread() {
    if (threadInitCallback_) //如果设置了就执行，在线程真正运行函数之前，进行一些初始化设置
    {
        threadInitCallback_();
    }
    while (running_) //如果running为true，则在这个循环里执行线程
    {
        Task task; //获取任务
        taskQueue_.waitAndPop(task);
        if (task)
        {
            task(); //执行该任务
        }
    }
}
