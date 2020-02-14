// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/sserver/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include "HttpServer.h"

#include "../../sserver_base/Logging.h"
#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#include <functional>

using namespace sserver;
using namespace sserver::net;

namespace sserver
{
namespace net
{
namespace detail
{
//http服务器类的封装
// FIXME: move to HttpContext class
bool processRequestLine(const char *begin, const char *end, HttpContext *context)
{
    bool succeed = false;
    const char *start = begin;
    const char *space = std::find(start, end, ' ');      //根据协议格式，先查找空格所在位置
    HttpRequest &request = context->request();           //取出请求对象
    if (space != end && request.setMethod(start, space)) //解析请求方法
    {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end)
        {
            const char *question = std::find(start, space, '?');
            if (question != space)
            {
                request.setPath(start, question);
                request.setQuery(question, space);
            }
            else
            {
                request.setPath(start, space); //解析path
            }
            start = space + 1;
            succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
            if (succeed)
            {
                if (*(end - 1) == '1') //判断是http1.1
                {
                    request.setVersion(HttpRequest::kHttp11);
                }
                else if (*(end - 1) == '0') //判断是http1.0
                {
                    request.setVersion(HttpRequest::kHttp10);
                }
                else
                {
                    succeed = false;
                }
            }
        }
    }
    return succeed;
}

// FIXME: move to HttpContext class
// return false if any error
bool parseRequest(Buffer *buf, HttpContext *context, Timestamp receiveTime)
{
    bool ok = true;
    bool hasMore = true;
    while (hasMore) //相当与一个状态机
    {
        if (context->expectRequestLine()) //处于解析请求行状态
        {
            const char *crlf = buf->findCRLF(); //这些数据都保存到缓冲区当中，在缓冲区寻找\r\n，头部每一行都有一个\r\n
            if (crlf)
            {
                ok = processRequestLine(buf->peek(), crlf, context); //解析请求行
                if (ok)
                {
                    context->request().setReceiveTime(receiveTime); //设置请求时间
                    buf->retrieveUntil(crlf + 2);                   //将请求行从buf中取回，包括\r\n，所以要+2
                    context->receiveRequestLine();                  //httpcontext将状态改为kexpectheaders
                }
                else
                {
                    hasMore = false;
                }
            }
            else
            {
                hasMore = false;
            }
        }
        else if (context->expectHeaders()) //处于解析header的状态
        {
            const char *crlf = buf->findCRLF();
            if (crlf)
            {
                const char *colon = std::find(buf->peek(), crlf, ':'); //查找冒号所在位置
                if (colon != crlf)
                {
                    context->request().addHeader(buf->peek(), colon, crlf);
                }
                else
                {
                    // empty line, end of header
                    context->receiveHeaders(); //httpcontext将状态改为kgotall
                    hasMore = !context->gotAll();
                }
                buf->retrieveUntil(crlf + 2); //将header从buf中取回，包括\r\n
            }
            else
            {
                hasMore = false;
            }
        }
        else if (context->expectBody()) //当前还暂时不支持带body，需要补充
        {
            // FIXME:
        }
    }
    return ok;
}

void defaultHttpCallback(const HttpRequest &, HttpResponse *resp)
{
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}

} // namespace detail
} // namespace net
} // namespace sserver

HttpServer::HttpServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &name,
                       TcpServer::Option option)
    : server_(loop, listenAddr, name, option),
      httpCallback_(detail::defaultHttpCallback)
{
    server_.setConnectionCallback( //注册这两个回调函数
        std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(
        std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

HttpServer::~HttpServer()
{
}

void HttpServer::start()
{
    LOG_WARN << "HttpServer[" << server_.name()
             << "] starts listenning on " << server_.hostport();
    server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        conn->setContext(HttpContext()); //tcpconnection与一个httpcontext绑定
    }
}

void HttpServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buf,
                           Timestamp receiveTime)
{
    HttpContext *context = std::any_cast<HttpContext>(conn->getMutableContext()); //获取的是可以改变的

    if (!detail::parseRequest(buf, context, receiveTime)) //获取请求包，更好的做法是让parserequest作为httpcontext的成员函数
    {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n"); //请求失败
        conn->shutdown();
    }
    //请求消息解析完毕
    if (context->gotAll())
    {
        onRequest(conn, context->request()); //连接和请求对象传过来
        context->reset();                    //本次请求处理完毕，重置httpcontext，适用于长连接
    }
}

void HttpServer::onRequest(const TcpConnectionPtr &conn, const HttpRequest &req)
{
    const string &connection = req.getHeader("Connection");                                //把头部取出来
    bool close = connection == "close" ||                                                  //如果等于close
                 (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive"); //或者是1.0且connection不等与keep-alive，http1.0不支持长连接
    HttpResponse response(close);                                                          //处理完请求是否要关闭连接
    httpCallback_(req, &response);                                                         //回调用户的函数对http请求进行相应处理，一旦处理完了返回response对象，是一个输入输出参数
    Buffer buf;
    response.appendToBuffer(&buf);  //将对象转换为一个字符串转换到buf中
    conn->send(&buf);               //将缓冲区发送个客户端
    if (response.closeConnection()) //如果需要关闭，短连接
    {
        conn->shutdown(); //关闭
    }
}
