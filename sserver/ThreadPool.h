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
{//实现的是固定线程池，不考虑动态的伸缩
 public:
  typedef function<void ()> Task;
  
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool(const ThreadPool&) = delete; //禁止拷贝构造

  explicit ThreadPool(const string& name = string("ThreadPool"));
  ~ThreadPool();

  // Must be called before start().
  void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
  void setThreadInitCallback(const Task& cb)
  { threadInitCallback_ = cb; }

  void start(int numThreads);
  void stop();

  // Could block if maxQueueSize > 0
  void run(const Task& f);
  void run(Task&& f);

 private:
  bool isFull() const;
  void runInThread();//运行线程
  Task take();

  MutexLock mutex_;
  Condition notEmpty_;
  Condition notFull_;
  string name_;
  Task threadInitCallback_;
  boost::ptr_vector<sserver::Thread> threads_;
  std::deque<Task> queue_;
  size_t maxQueueSize_;
  bool running_;
};

}

#endif
