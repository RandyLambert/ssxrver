#include <cstdio>
#include "HttpResponse.h"
#include "../net/Buffer.h"
using namespace ssxrver;
using namespace ssxrver::net;

void HttpResponse::appendToBuffer(Buffer *output) const
{                 //http响应类的封装
    char buf[32]; //添加响应头
    snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", statusCode_);
    output->append(buf);
    output->append(statusMessage_); //添加响应信息
    output->append("\r\n");

    if (closeConnection_)
    {
        //如果是短连接，不需要告诉浏览器content-length，浏览器也能正常处理
        //短连接不惜要告诉包的长度，因为他处理完就直接断开了，所以不存在粘包问题
        output->append("TcpConnection: close\r\n");
    }
    else
    {
        if(file_ == nullptr)
            snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.size());
        else
            snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.size() + static_cast<unsigned long>(file_->getSendLen())); //如果是长连接才需要这一行头部信息，来说明包的实体长度
        output->append(buf);
        output->append("TcpConnection: Keep-Alive\r\n"); //长连接的标志
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
}

void HttpResponse::swap(HttpResponse &that)
{
    file_.swap(that.file_);
    headers_.swap(that.headers_);
    std::swap(statusCode_,that.statusCode_);
    statusMessage_.swap(that.statusMessage_);
    std::swap(closeConnection_,that.closeConnection_);
    body_.swap(that.body_);
}
