#ifndef SSXRVER_BASE_LOGSTREAM_H
#define SSXRVER_BASE_LOGSTREAM_H
#include <string.h>
#include "noncopyable.h"
namespace ssxrver
{
namespace detail
{
const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template <int SIZE>
    class FixedBuffer : ssxrver::noncopyable
{
    FixedBuffer()
        :cur_(data_)
    {
    }

    ~FixedBuffer() = default;

    void append(const char * buf, size_t len)
    {
        if (static_cast<size_t>(avail()) > len) //当前可用的空间大于len，则就可以将其添加进去
        {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
        else
        { //如果小于，只要缓冲区不等于0，则将部分大小放入缓冲区，但是最后一位不能占，因为可能会放结束符'\0'
            if (static_cast<size_t>(avail()) > 0)
            {
                memcpy(cur_, buf, static_cast<size_t>(avail()) - 1);
                cur_ += (static_cast<size_t>(avail()) - 1);
            }
        }
    }

    const char *data() const { return data_; } //首地址，current-data是缓冲区所有的数据
    int length() const { return static_cast<int>(cur_ - data_); }

    char *current() { return cur_; } //目前缓冲区使用的位置，end-current是当前可用空间
    int avail() const { return static_cast<int>(end() - cur_); }

    void reset() {cur_ = data_;} //重置，只要把指针移到开头，不需要清0
    void bzero() { ::bzero(data_,sizeof(data_));}

private:
    const char *end() const { return data_ + sizeof(data_); } //整个缓冲区size位的下一位
    // Must be outline function for cookies.
    static void cookieStart();
    static void cookieEnd();

    char data_[SIZE];  //缓冲区大小通过模板传递
    char *cur_;        //指向缓冲区最后一个位置的下一个
};
}

class LogStream
{
public:
    LogStream() {}
    ~LogStream() {}

private:

};

}
#endif
