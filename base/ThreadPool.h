//
// Created by randylambert on 2020/11/6.
//

#ifndef SSXRVER_THREADPOOL_H
#define SSXRVER_THREADPOOL_H
#include <boost/noncopyable.hpp>
#include <memory>
#include <utility>
#include <vector>
#include <functional>
#include "Thread.h"
#include "ThreadSafeQueue.hpp"
namespace ssxrver::base {

class ThreadPool : boost::noncopyable
{
public:
    using Task = std::function<void()>;
    explicit ThreadPool();
    ~ThreadPool();
    void setThreadInitCallback(Task cb) { threadInitCallback_ = std::move(cb); }
    void start(unsigned numThreads);
    void stop();
    void addTask(Task task) {taskQueue_.push(std::move(task));};
private:
    void runInThread();  //线程池的线程运行函数
    Task threadInitCallback_;
    bool running_;
    std::vector<std::unique_ptr<Thread>> threads_;
    ThreadSafeQueue<Task> taskQueue_;
};

}


#endif //SSXRVER_THREADPOOL_H
