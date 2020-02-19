#ifndef SSXRVER_BASE_EXCEPTION_H
#define SSXRVER_BASE_EXCEPTION_H
#include <exception>
#include <string>
namespace ssxrver
{
using std::string;
class Exception : public std::exception//继承标准类可以写异常类
{
public:
    explicit Exception(const char *what) : message_(what){ fillStackTrace();}
    explicit Exception(const string& what) : message_(what){ fillStackTrace();}
    virtual ~Exception() noexcept = default;
    virtual const char *what() const noexcept {return message_.c_str();}
    const char *stackTrace() const noexcept{return stack_.c_str();}

private:
    void fillStackTrace();
    string demangle(const char *symbol);
    string message_; //存错误跑出的内容
    string stack_;   //存调用的函数栈

};
}
#endif
