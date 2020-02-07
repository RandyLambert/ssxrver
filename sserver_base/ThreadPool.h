#ifndef SSERVER_THREADPOOL_H
#define SSERVER_THREADPOOL_H

#include "Condition.h"
#include "Mutex.h"
#include "Thread.h"
#include <functional>
#include <boost/ptr_container/ptr_vector.hpp>
#include <deque>

namespace sserver
{

class ThreadPool
{ //实现的是固定线程池，不考虑动态的伸缩
public:
  typedef function<void()> Task; //函数接口

  ThreadPool &operator=(const ThreadPool &) = delete;
  ThreadPool(const ThreadPool &) = delete; //禁止拷贝构造

  explicit ThreadPool(const string &name = string("ThreadPool"));
  ~ThreadPool();

 
  // Must be called before start().因为在start()调用时，会从队列取任务
  void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; } //设置任务队列的大小，必须在start之前设置。
  void setThreadInitCallback(const Task &cb)                     //设置回调函数，每次在执行任务前先调用回调函数
  {
    threadInitCallback_ = cb;
  }
  
  
  void start(int numThreads); //启动线程池,numThreads是线程池的容量
  void stop();                //终止线程池

  const string &name() const
  {
    return name_;
  }

  size_t queueSize() const;

  // Could block if maxQueueSize > 0
  void run(const Task &f); //传引用是为了高效，但是又不想改值，所以就加const
  void run(Task &&f);      //把任务添加到任务队列，可能不是立即执行。

private:
  bool isFull() const; //判满
  void runInThread();  //线程池的线程运行函数
  Task take();         //取任务函数

  mutable MutexLock mutex_; //mutable表示在const函数也可以改变它
  Condition notEmpty_;      //任务队列queue_不为空了，有任务可以执行了，进而唤醒等待的线程。
  Condition notFull_;       //任务队列queue_不满了，有空间可以使用了，进而唤醒等待的线程。
  string name_;
  Task threadInitCallback_;                    //线程初始化回调函数,在线程池第一次执行任务是调用。
  boost::ptr_vector<sserver::Thread> threads_; //工作线程容器(线程数组),存放线程
  std::deque<Task> queue_;                     //任务队列
  size_t maxQueueSize_;                        //任务队列最大大小
  bool running_;                               //线程池是否运行
};

} // namespace sserver

#endif
