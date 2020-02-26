#ifndef SSXRVER_BASE_LOGSTREAM_H
#define SSXRVER_BASE_LOGSTREAM_H
#include <string>
#include <cstring>
#include <cassert>
#include "noncopyable.h"
namespace ssxrver
{
namespace detail
{
const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template <int SIZE>
class FixedBuffer : noncopyable
{
public:
    FixedBuffer()
        :cur_(data_)
    {
    }

    ~FixedBuffer() = default;

    void append(const char * buf, size_t len)
    {
        if (avail() > len) //当前可用的空间大于len，则就可以将其添加进去
        {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
        else
        { //如果小于，只要缓冲区不等于0，则将部分大小放入缓冲区，但是最后一位不能占，因为可能会放结束符'\0'
            if (avail() > 0)
            {
                memcpy(cur_, buf, avail() - 1);
                cur_ += (avail() - 1);
            }
        }
    }

    const char *data() const { return data_; } //首地址，current-data是缓冲区所有的数据
    size_t length() const { return static_cast<size_t>(cur_ - data_); }
    void add(size_t len) { cur_ += len; }
    char *current() { return cur_; } //目前缓冲区使用的位置，end-current是当前可用空间
    size_t avail() const { return static_cast<size_t>(end() - cur_); }

    void reset() {cur_ = data_;} //重置，只要把指针移到开头，不需要清0
    void bzero() { ::bzero(data_,sizeof(data_));}

private:
    const char *end() const { return data_ + sizeof(data_); } //整个缓冲区size位的下一位
    // Must be outline function for cookies.

    char data_[SIZE];  //缓冲区大小通过模板传递
    char *cur_;        //指向缓冲区最后一个位置的下一个
};
}

class LogStream : noncopyable
{
    typedef LogStream self;
public:
    typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;
    self& operator<<(bool s)
    {
        buffer_.append(s?"1":"0",1);
        return *this;
    }
    self &operator<<(short); //处理所有类型
    self &operator<<(unsigned short);
    self &operator<<(int);
    self &operator<<(unsigned int);
    self &operator<<(long);
    self &operator<<(unsigned long);
    self &operator<<(long long);
    self &operator<<(unsigned long long);

    self &operator<<(const void *);

    self &operator<<(float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }
    self &operator<<(double);
    self &operator<<(char v)
    {
        buffer_.append(&v, 1);
        return *this;
    }

    self &operator<<(const char *str)
    {
        if (str)
        {
            buffer_.append(str, strlen(str));
        }
        else
        {
            buffer_.append("(null)", 6);
        }
        return *this;
    }

    self &operator<<(const unsigned char *str)
    {
        return operator<<(reinterpret_cast<const char *>(str));
    }

    self &operator<<(const std::string &v)
    {
        buffer_.append(v.c_str(),v.size());
        return *this;
    }
    void append(const char *data,int len) { buffer_.append(data,len); }
    const Buffer &buffer() const { return buffer_; }
    void resetBuffer() { buffer_.reset(); }

private:
    Buffer buffer_;

    template <typename T>
    void formatInteger(T); //成员模板

    static const int kMaxNumericSize = 21;
};

}
#endif
