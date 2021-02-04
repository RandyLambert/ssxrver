#ifndef SSXRVER_BASE_THREAD_H
#define SSXRVER_BASE_THREAD_H
#include <thread>
#include <string>
#include <functional>
#include <atomic>
#include <boost/noncopyable.hpp>
#include <boost/thread/latch.hpp>
namespace ssxrver
{

    class Thread : boost::noncopyable
    {
    public:
        using ThreadFunc = std::function<void()>;
        explicit Thread(ThreadFunc,std::string name = "DefaultThread");
        ~Thread();

        void start(); //初始化
        [[nodiscard]] bool started() const { return started_; }
        const std::string &name() { return name_; }

    private:

        bool started_;
        std::unique_ptr<std::thread> thread_;
        pid_t tid_;           //该线程在计算机中的唯一标识
        ThreadFunc func_;     //函数接口
        std::string name_;
        boost::latch latch_;

    };

} // namespace ssxrver

#endif