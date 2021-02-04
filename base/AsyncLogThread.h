#ifndef SSXRVER_BASE_ASYNCLOGTHREAD_H
#define SSXRVER_BASE_ASYNCLOGTHREAD_H
#include <vector>
#include <string>
#include <functional>
#include <condition_variable>
#include <memory>
#include <boost/thread/latch.hpp>
#include "Thread.h"
#include "LogStream.h"

namespace ssxrver::base
{

class AsyncLogThread : boost::noncopyable
{
public:
    explicit AsyncLogThread(std::string basename, int flushSecond = 3,size_t rollSize = 64*1024);
    ~AsyncLogThread()
    {
        if (running_)
            stop();
    }

    void append(const char *log_, size_t len);
    void start();
    void stop();

private:
    void threadFunc();
    typedef detail::FixedBuffer<detail::kLargeBuffer> Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
    typedef std::unique_ptr<Buffer> BufferPtr;

    const int flushSecond_;
    const size_t rollSize_;
    bool running_;
    std::string basename_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
    boost::latch latch_;
};
} // namespace ssxrver::base


#endif
