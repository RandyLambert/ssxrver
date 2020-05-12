#ifndef SSXRVER_BASE_ASYNCLOGTHREAD_H
#define SSXRVER_BASE_ASYNCLOGTHREAD_H
#include <vector>
#include <string>
#include <functional>
#include "Condition.h"
#include "Thread.h"
#include "Mutex.h"
#include "Thread.h"
#include "CountDownLatch.h"
#include "LogStream.h"

#include <vector>
#include <memory>
namespace ssxrver
{
namespace base
{

class AsyncLogThread : noncopyable
{
public:
    AsyncLogThread(const std::string basename, int flushInterval = 2);
    ~AsyncLogThread()
    {
        if (running_)
            stop();
    }

    void append(const char *log_, int len);
    void start();
    void stop();

private:
    void threadFunc();
    typedef detail::FixedBuffer<detail::kLargeBuffer> Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
    typedef std::unique_ptr<Buffer> BufferPtr;

    const int flushInterval_;
    bool running_;
    std::string basename_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
    CountDownLatch latch_;
};
} // namespace base
} // namespace ssxrver

#endif
