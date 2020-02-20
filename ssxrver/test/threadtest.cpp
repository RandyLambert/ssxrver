#include "../base/CurrentThread.h"
#include "../base/Mutex.h"
#include "../base/Thread.h"

#include <boost/bind.hpp>

using namespace ssxrver;
void threadFunc()
{
  printf("tid=%d\n", ssxrver::CurrentThread::tid());
}

int main()
{
  printf("pid=%d, tid=%d\n", ::getpid(), ::CurrentThread::tid());

  int kThreads = 10;
  for (int i = 0; i < kThreads; ++i)
  {
    ssxrver::Thread t1(threadFunc);
    t1.start();
    t1.join();
  }

  printf("number of created threads %d\n", ssxrver::Thread::numCreated());

  for (int i = 0; i < kThreads; ++i)
  {
    ssxrver::Thread t2(boost::bind(threadFunc));
    t2.start();
    t2.join();
  }
}
