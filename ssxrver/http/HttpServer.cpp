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

}
}
}

HttpServer::HttpServer(EventLoop *loop,
                       const struct sockaddr_in listenAddr)
    :server_(loop,listenAddr),
    httpCallback_(detail::defaultHttpCallback)
{
    /* server_.setMessageCallback() */

}
