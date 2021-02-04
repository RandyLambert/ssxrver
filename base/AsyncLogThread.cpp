#include "AsyncLogThread.h"

#include <memory>
#include "LogFile.h"
using namespace ssxrver;
using namespace ssxrver::base;
AsyncLogThread::AsyncLogThread(std::string basename, int flushSecond,size_t rollSize)
    : flushSecond_(flushSecond),
      rollSize_(rollSize),
      running_(false),
      basename_(std::move(basename)),
      thread_([this] { threadFunc(); }, "AsyncLogThread"),
      mutex_(),
      cond_(),
      currentBuffer_(std::make_unique<Buffer>()),
      nextBuffer_(std::make_unique<Buffer>()),
      buffers_(),
      latch_(1)
{
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}

void AsyncLogThread::start()
{
    running_ = true;
    thread_.start();
    latch_.wait();
}

void AsyncLogThread::stop()
{
    running_ = false;
    cond_.notify_one();
}

void AsyncLogThread::append(const char *log_, size_t len)
{
    std::unique_lock <std::mutex> lock(mutex_);
    if (currentBuffer_->avail() > static_cast<size_t>(len))
        currentBuffer_->append(log_, len);
    else
    {
        buffers_.push_back(std::move(currentBuffer_));
        if (nextBuffer_)
            currentBuffer_ = std::move(nextBuffer_);
        else
            currentBuffer_ = std::make_unique<Buffer>();
        currentBuffer_->append(log_, len);
        cond_.notify_one();
    }
}

void AsyncLogThread::threadFunc()
{
    assert(running_);
    latch_.count_down();
    file::LogFile output(basename_,rollSize_);
    BufferPtr newBuffer1 = std::make_unique<Buffer>();
    BufferPtr newBuffer2 = std::make_unique<Buffer>();

    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);

    while (running_)
    {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        {
            std::unique_lock <std::mutex> lock(mutex_);
            if (buffers_.empty())
            {
                cond_.wait_for(lock,std::chrono::seconds(flushSecond_));
            }
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(newBuffer1);
            buffersToWrite.swap(buffers_);

            if (!nextBuffer_)
            {
                nextBuffer_ = std::move(newBuffer2);
            }
        }

        assert(!buffersToWrite.empty());
        if (buffersToWrite.size() > 25)
        {
            buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
        }

        for (const auto &buffer : buffersToWrite)
        {
            output.append(buffer->data(), buffer->length());
        }

        if (buffersToWrite.size() > 2)
        {
            buffersToWrite.resize(2);
        }

        if (!newBuffer1)
        {
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (!newBuffer2)
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
        output.flush(); //刷新条件1，到达超时时间，刷新条件2，有某个缓冲区写满了数据
    }
    output.flush();
}
