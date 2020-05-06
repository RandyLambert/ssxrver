#include <execinfo.h>
#include <cxxabi.h>
#include <stdlib.h>
#include "Exception.h"
using namespace ssxrver;

void Exception::fillStackTrace()
{
    enum
    {
        kSize = 100
    };
    void *buffer[kSize];
    int nptrs = ::backtrace(buffer, kSize); //获取当前线程的函数调用堆栈。
    /* int backtrace(void **buffer, int size); */
    /* 返回调用堆栈 */
    /* buffer :提供一个指针的数组 */
    /* size :指定缓冲区的个数，即设置的调用深度 */
    /* int : 返回实际返回的调用深度 */
    /* 每个地址指针由 函数名、地址偏移、返回地址组成 */

    char **strings = ::backtrace_symbols(buffer, nptrs); //malloc申请的，需要我们释放
    /* backtrace_symbols将从backtrace函数获取的信息转化为一个字符串数组. */
    /* 参数buffer应该是从backtrace函数获取的数组指针,size是该数组中的 */
    /* 元素个数(backtrace的返回值)，函数返回值是一个指向字符串数组的指针 */
    /* ,它的大小同buffer相同.每个字符串包含了一个相对于buffer中对应元 */
    /* 素的可打印信息.它包括函数名，函数的偏移地址,和实际的返回地址 */

    if (strings)
    { //处理函数名，使函数名写进stack_中
        for (int i = 0; i < nptrs; ++i)
        {
            stack_.append(demangle(strings[i])); //本来是带c++改过的函数名，是编译器看到的函数名，现在用demangle函数去掉了
            stack_.push_back('\n');
        }
        free(strings);
    }
}

string Exception::demangle(const char *symbol)
{
    enum
    {
        kSize = 128
    };
    size_t size;
    int status;
    char temp[kSize];
    char *demangled;
    //尝试去demandle c++ name
    if (1 == sscanf(symbol, "%*[^(]%*[^_]%127[^)+]", temp))
    {
        if (NULL != (demangled = abi::__cxa_demangle(temp, NULL, &size, &status)))
        {
            string result(demangled);
            free(demangled);
            return result;
        }
    }
    //如果不行，用标准c的符号
    if (1 == sscanf(symbol, "%127s", temp))
    {
        return temp;
    }

    //都不行
    return symbol;
}
