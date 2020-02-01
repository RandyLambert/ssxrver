#ifndef SSERVER_CURRENTTHREAD_H
#define SSERVER_CURRENTTHREAD_H

#include <stdint.h>

namespace sserver
{
namespace CurrentThread
{
  extern __thread int t_cachedTid;
  extern __thread char t_tidString[32];
  extern __thread int t_tidStringLength;
  extern __thread const char* t_threadName;
  void cacheTid();

  inline int tid()
  {
    if (t_cachedTid == 0) //说明线程还没有缓存过数据
    {
      cacheTid(); //通过系统调用缓存得到线程pid
    }
    return t_cachedTid;
  }

  inline const char* tidString() // for logging
  {
    return t_tidString;
  }

  inline int tidStringLength() // for logging
  {
    return t_tidStringLength;
  }

  inline const char* name()
  {
    return t_threadName;
  }

  bool isMainThread();

  void sleepUsec(int64_t usec);
}
}

#endif

