#include <stdio.h>
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
        output->append("Connection: close\r\n");
    }
    else
    {
        snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.size()); //如果是长连接才需要这一行头部信息，来说明包的实体长度
        output->append(buf);
        output->append("Connection: Keep-Alive\r\n"); //长连接的标志
    }

    //header列表
    for (const std::pair<string, string> &header : headers_)
    {
        output->append(header.first);
        output->append(": ");
        output->append(header.second);
        output->append("\r\n");
    }

    output->append("\r\n"); //header与body之间的空行
    output->append(body_);
}
