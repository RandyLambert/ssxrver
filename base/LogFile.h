#ifndef SSXRVER_BASE_LOGFILE_H
#define SSXRVER_BASE_LOGFILE_H
#include <memory>
#include <mutex>
#include <string_view>
#include <boost/noncopyable.hpp>
namespace ssxrver::base::file
{

class writeFile;
class LogFile : boost::noncopyable
{
public:
    explicit LogFile(std::string_view baseName, size_t rollSize = 1024 * 64, bool threadSafe = true);
    ~LogFile();
    void append(const char *log_, size_t len); //添加日志
    void flush();
    void rollFile(); //滚动日志

private:
    void getLogFileName(std::string &name);
    void append_unlocked(const char *log_, size_t len); //不加锁的添加
    const std::string baseName_;
    int count_; //暂时用不上
    size_t rollSize_;
    std::unique_ptr<std::mutex> mutex_;
    std::unique_ptr<file::writeFile> file_;

    const static int kFlushInterval_ = 5; // 间隔刷新次数，暂时用不上，因为使用文件的时候只是在异步日志的时候用，但是当使用异步日志等等时候，
                                        // 在超时时间之后或者是缓冲区满了之后会刷新，如果不用异步日志模块的话，默认是给标准输出，也没用到LogFile
};

} // namespace ssxrver::base::file

#endif
