#include <functional>
#include "HttpServer.h"
#include "../base/Logging.h"
#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
using namespace ssxrver;
using namespace ssxrver::net;

namespace ssxrver
{
namespace net
{
namespace detail
{
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
            succeed = end - start == 8 && std::equal(start, end, "HTTP/1.1");
        }
    }
    return succeed;
}

bool parseRequest(Buffer *buf, HttpContext *context)
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
                    // 空行，头解析完毕
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

}
}
}

HttpServer::HttpServer(EventLoop *loop,
                       const struct sockaddr_in listenAddr)
    :server_(loop,listenAddr),
    httpCallback_(detail::defaultHttpCallback)
{
    server_.setMessageCallback(std::bind(&HttpServer::onMessage,this,std::placeholders::_1,std::placeholders::_2));
    server_.setConnectionCallback(std::bind(&HttpServer::onConnection,this,std::placeholders::_1));
}

void HttpServer::start()
{
    LOG_WARN << "HttpServer on";
    server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr &conn)
{
    if(conn->connected())
        conn->setContext(HttpContext());
}

void HttpServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buf)
{
    HttpContext *context = boost::any_cast<HttpContext>(conn->getMutableContext()); //获取的是可以改变的
    if (!detail::parseRequest(buf, context)) //获取请求包，更好的做法是让parserequest作为httpcontext的成员函数
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

void HttpServer::onRequest(const TcpConnectionPtr &conn,const HttpRequest &req)
{
    const string &connection = req.getHeader("Connection");
    bool close;
    if(connection == "close")
        close = true;
    else
        close = false;
    HttpResponse response(close);
    httpCallback_(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf); //将对象转化为一个字符串到buf中
    conn->send(&buf);              //将缓冲区发送到客户端
    if(response.closeConnection()) //如果需要关闭，短连接
        conn->shutdown();
}
