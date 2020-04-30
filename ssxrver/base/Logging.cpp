#include <sstream>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include "Logging.h"
#include "CurrentThread.h"
namespace ssxrver
{

__thread char t_errnobuf[512];
const char *strerror_tl(int savedErrno)
{
    return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}

Logger::LogLevel initLogLevel()
{
    return Logger::INFO;
}
Logger::LogLevel g_logLevel = initLogLevel();

const char *LogLevelName[Logger::NUM_LOG_LEVELS] =
    {
        "TRACE ",
        "DEBUG ",
        "INFO  ",
        "WARN  ",
        "ERROR ",
        "FATAL ",
};

class T
{
public:
    T(const char *str, unsigned len)
        : str_(str),
          len_(len)
    {
        assert(strlen(str) == len_);
    }

    const char *str_;
    const unsigned len_;
};

LogStream &operator<<(LogStream &s, T v)
{
    s.append(v.str_, v.len_);
    return s;
}

LogStream &operator<<(LogStream &s, const Logger::SourceFile &v)
{
    s.append(v.data_, v.size_);
    return s;
}

void defaultOutput(const char *msg, int len)
{
    size_t n = fwrite(msg, 1, len, stdout);
    (void)n;
}

void defaultFlush()
{
    fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

} // namespace ssxrver

using namespace ssxrver;

Logger::Impl::Impl(LogLevel level, int savedErrno, const SourceFile &file, int line)
    : stream_(),
      level_(level),
      line_(line),
      basename_(file)
{
    CurrentThread::tid(); //当前线程的tid
    stream_ << CurrentThread::tidString() << " ";
    stream_ << CurrentThread::tidStringLength();
    stream_ << LogLevelName[level];
    if (savedErrno != 0) //在把级别格式化进去
    {
        stream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
    }
}

void Logger::Impl::finish()
{
    stream_ << " - " << basename_.data_ << ':' << line_ << '\n';
}

Logger::Logger(SourceFile file, int line)
    : impl_(INFO, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, LogLevel level, const char *func)
    : impl_(level, 0, file, line) //嵌套类构造
{
    impl_.stream_ << func << ' '; //函数名称在此格式化
}

Logger::Logger(SourceFile file, int line, LogLevel level)
    : impl_(level, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, bool toAbort)
    : impl_(toAbort ? FATAL : ERROR, errno, file, line)
{
}

Logger::~Logger()
{
    impl_.finish();                                  //结束
    const LogStream::Buffer &buf(stream().buffer()); //把缓冲区取出来，保存到buf里
    g_output(buf.data(), buf.length());              //然后输出，默认是stdout
    if (impl_.level_ == FATAL)                       //级别高
    {
        g_flush(); //刷新缓冲区，到文件里
        abort();   //退出
    }
}

void Logger::setLogLevel(Logger::LogLevel level)
{
    g_logLevel = level;
}

void Logger::setOutput(OutputFunc out)
{
    g_output = out; //要修改输出的位置，更改为新的输出，默认是stdout
}

void Logger::setFlush(FlushFunc flush)
{
    g_flush = flush; //刷新流，把缓冲区的全要输出
}
