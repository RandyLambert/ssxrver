#include "LogFile.h"
#include "File.h"
#include <time.h>
using namespace ssxrver;
using namespace ssxrver::base::file;
using std::string;
LogFile::LogFile(const string &baseName, size_t rollSize, bool threadSafe)
    : baseName_(baseName),
      count_(0),
      rollSize_(rollSize),
      mutex_(threadSafe ? new MutexLock : NULL)
{
    rollFile();
}

LogFile::~LogFile() = default;

void LogFile::rollFile()
{
    string fileName;
    getLogFileName(fileName);
    file_.reset(new writeFile(fileName));
}

void LogFile::getLogFileName(string &name)
{
    time_t now = 0;
    struct tm tm;
    now = time(NULL);
    gmtime_r(&now, &tm);
    char timebuf[32];
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
    name = baseName_ + timebuf;
    name += "log";
}

void LogFile::append(const char *log_, int len) //添加日志
{
    if (mutex_)
    {
        MutexLockGuard lock(*mutex_);
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
        MutexLockGuard lock(*mutex_);
        file_->flush();
    }
    else
    {
        file_->flush();
    }
}
void LogFile::append_unlocked(const char *log_, int len) //不加锁的添加
{
    file_->append(log_, len);
    count_++;
    if (count_ >= kFlushInterval_)
    {
        flush();
        count_ = 0;
    }
    if (file_->writeLen() >= rollSize_)
        rollFile();
}
