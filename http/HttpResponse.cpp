#include <cstdio>
#include "HttpResponse.h"
#include "../net/Buffer.h"
using namespace ssxrver;
using namespace ssxrver::net;

void HttpResponse::appendToBuffer(Buffer *output) const
{                 //http响应类的封装
    char buf[32]; //添加响应头
    if(version_ == 0x11)
    {
        snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", statusCode_);
    }
    else if(version_ == 0x10)
    {
        snprintf(buf, sizeof(buf), "HTTP/1.0 %d ", statusCode_);
    }
    output->append(buf);
    output->append(statusMessage_); //添加响应信息
    output->append("\r\n");

    if (closeConnection_)
    {
        //如果是短连接，不需要告诉浏览器content-length，浏览器也能正常处理
        //短连接不惜要告诉包的长度，因为他处理完就直接断开了，所以不存在粘包问题
        output->append("Connection: close\r\n");
    }
    else
    {
        snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.size()); //如果是长连接才需要这一行头部信息，来说明包的实体长度
        output->append(buf);
        output->append("Connection: Keep-Alive\r\n"); //长连接的标志
    }

    //header列表
    for (const auto &header : headers_)
    {
        output->append(header.first);
        output->append(": ");
        output->append(header.second);
        output->append("\r\n");
    }

    output->append("\r\n"); //header与body之间的空行
    output->append(body_);
    // std::cout << output->retrieveAllAsString() << std::endl;
}
bool HttpResponse::setVersion(const char *start, const size_t length)
{
    assert(version_ == 0x00);
    if (strncmp(start, "HTTP/1.1", length) == 0)
    {
        version_ = 0x11;
    }
    else if (strncmp(start, "HTTP/1.0", length) == 0)
    {
        version_ = 0x10;
    }
    else
    {
        version_ = 0x00;
    }
    return version_ != 0x00;
}

void HttpResponse::swap(HttpResponse &that)
{
    headers_.swap(that.headers_);
    std::swap(statusCode_,that.statusCode_);
    statusMessage_.swap(that.statusMessage_);
    std::swap(closeConnection_,that.closeConnection_);
    body_.swap(that.body_);
    std::swap(version_,that.version_);
}
