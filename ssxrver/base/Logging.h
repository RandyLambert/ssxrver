#ifndef SSXRVER_BASE_LOGGING_H
#define SSXRVER_BASE_LOGGING_H
#include "LogStream.h"
namespace ssxerver
{
class Logger
{
public:
    enum LogLevel
    {
        TRACE,          //指出比DEBUG粒度更细的一些信息事件(开发时使用较多)
        DEBUG,          //指出细粒度信息事件对调试应用程序是非常有帮助的(开发是使用较多)
        INFO,           //表明消息在粗粒度级别上突出强调应用程序的运行过程
        WARN,           //系统能正常运行，但是可能会出现潜在错误的情形
        ERROR,          //值虽然发生错误事件，但是仍然不影响系统的继续运行
        FATAL,          //指出每个严重的错误时间将会直接导致应用程序的退出
        NUM_LOG_LEVELS, //级别个数
    };
};

}
#endif
