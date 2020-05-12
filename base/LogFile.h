#ifndef SSXRVER_BASE_LOGFILE_H
#define SSXRVER_BASE_LOGFILE_H
#include <memory>
#include "Mutex.h"
#include "noncopyable.h"
namespace ssxrver
{
namespace base
{
namespace file
{

class writeFile;
class LogFile : noncopyable
{
public:
    LogFile(const std::string &baseName, size_t rollSize = 1024 * 64, bool threadSafe = true);
    ~LogFile();
    void append(char *log_, int len); //添加日志
    void flush();
    void rollFile(); //滚动日志

private:
    void getLogFileName(std::string &name);
    void append_unlocked(const char *log_, int len); //不加锁的添加
    const std::string baseName_;
    int count_;
    size_t rollSize_;
    std::unique_ptr<MutexLock> mutex_;
    std::unique_ptr<file::writeFile> file_;

    const static int kFlushInterval_ = 6; //间隔刷新次数
};
} // namespace file
} // namespace base

} // namespace ssxrver

#endif
