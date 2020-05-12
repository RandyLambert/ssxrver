#ifndef SSXRVER_BASE_LOGGING_H
#define SSXRVER_BASE_LOGGING_H
#include "LogStream.h"
namespace ssxrver
{
class Logger
{
public:
    enum LogLevel
    {
        DEBUG,          //指出细粒度信息事件对调试应用程序是非常有帮助的(开发是使用较多)
        INFO,           //表明消息在粗粒度级别上突出强调应用程序的运行过程
        WARN,           //系统能正常运行，但是可能会出现潜在错误的情形
        ERROR,          //值虽然发生错误事件，但是仍然不影响系统的继续运行
        FATAL,          //指出每个严重的错误时间将会直接导致应用程序的退出
        NUM_LOG_LEVELS, //级别个数
    };

    class SourceFile //帮助获得文件名
    {
    public:
        template <int N>
        inline SourceFile(const char (&arr)[N])
            : data_(arr),
              size_(N - 1)
        {
            const char *slash = strrchr(data_, '/'); //查到最后一个“/”并返回位置
            if (slash)
            {
                data_ = slash + 1;
                size_ -= (data_ - arr);
            }
        }

        explicit SourceFile(const char *filename)
            : data_(filename)
        {
            const char *slash = strrchr(filename, '/');
            if (slash)
            {
                data_ = slash + 1;
            }
            size_ = strlen(data_);
        }

        const char *data_;
        size_t size_;
    };

    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, LogLevel level, const char *func);
    Logger(SourceFile file, int line, bool toAbort);
    ~Logger();

    LogStream &stream() { return impl_.stream_; }

    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    typedef void (*OutputFunc)(const char *msg, int len); //函数指针，下同
    typedef void (*FlushFunc)();
    static void setOutput(OutputFunc); //设置输出函数
    static void setFlush(FlushFunc);   //清空缓冲

private:
    //Loger类的内部嵌套类
    //Impl类主要是负责日志的格式化
    class Impl
    {
    public:
        typedef Logger::LogLevel LogLevel;
        Impl(LogLevel level, int old_errno, const SourceFile &file, int line);
        void finish(); //将日志写道缓冲区

        LogStream stream_; //LogStream类对象成员
        LogLevel level_;   //日志级别
        int line_;         //行号
        SourceFile basename_;
    };

    Impl impl_; //Impl类对象成员
    //类SouceFile用来确定日志文件的名字，而Impl是真正实现日志输出的地方。
    //在Logger类中可以设置Logger日志级别，以及设置缓存和清空缓存函数。
};

extern Logger::LogLevel g_logLevel;
inline Logger::LogLevel Logger::logLevel() //当前级别返回的是logLevel
{
    return g_logLevel;
}

//使用宏来定义匿名对象，LogStream重载了<<，因此可以使用 LOG_REACE<<"日志"<<
//日志输出宏，输出在哪个文件? 哪一行? 哪个函数? 哪种级别?
//无名对象所在语句执行后就立即被析构，然后调用析构函数将缓冲区的内容分输出
#define LOG_DEBUG                                              \
    if (ssxrver::Logger::logLevel() <= ssxrver::Logger::DEBUG) \
    ssxrver::Logger(__FILE__, __LINE__, ssxrver::Logger::DEBUG, __func__).stream()
#define LOG_INFO                                              \
    if (ssxrver::Logger::logLevel() <= ssxrver::Logger::INFO) \
    ssxrver::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN ssxrver::Logger(__FILE__, __LINE__, ssxrver::Logger::WARN).stream()
#define LOG_ERROR ssxrver::Logger(__FILE__, __LINE__, ssxrver::Logger::ERROR).stream()
#define LOG_FATAL ssxrver::Logger(__FILE__, __LINE__, ssxrver::Logger::FATAL).stream()
#define LOG_SYSERR ssxrver::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL ssxrver::Logger(__FILE__, __LINE__, true).stream()
//后两个是系统错误
//构造了一个Logger对象，重载了输入运算符，以流的机制运行
//构造了一个匿名对象，所以用完之后，就没有存在价值了，接着调用析构函数

const char *strerror_tl(int savedErrno);

} // namespace ssxrver

#endif
