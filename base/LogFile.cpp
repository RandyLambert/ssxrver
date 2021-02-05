#include "LogFile.h"
#include "File.h"
#include <ctime>
#include <memory>
using namespace ssxrver;
using namespace ssxrver::base::file;
using std::string;
LogFile::LogFile(std::string_view baseName, size_t rollSize, bool threadSafe)
    : baseName_(baseName),
      count_(0),
      rollSize_(rollSize),
      mutex_(threadSafe ? new std::mutex : nullptr)
{
    rollFile();
}

LogFile::~LogFile() = default;

void LogFile::rollFile()
{
    string fileName;
    getLogFileName(fileName);
    file_ = std::make_unique<WriteFile>(fileName);
}

void LogFile::getLogFileName(string &name)
{
    time_t now = 0;
    struct tm tm{};
    now = time(nullptr);
    gmtime_r(&now, &tm);
    char timebuf[32];
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
    name = baseName_ + timebuf;
    name += "log";
}

void LogFile::append(const char *log_, size_t len) //添加日志
{
    if (mutex_)
    {
        std::scoped_lock<std::mutex> locker(*mutex_);
        append_unlocked(log_, len);
    }
    else
    {
        append_unlocked(log_, len);
    }
}
void LogFile::flush()
{
    if (mutex_)
    {
        std::scoped_lock<std::mutex> locker(*mutex_);
        file_->flush();
    }
    else
    {
        file_->flush();
    }
}
void LogFile::append_unlocked(const char *log_, size_t len) //不加锁的添加
{
    file_->append(log_, len);
//    count_++;
//    if (count_ >= kFlushInterval_)
//    {
//        flush();
//        count_ = 0;
//    }
    if (file_->writeLen() >= rollSize_)
        rollFile();
}
