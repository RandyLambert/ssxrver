#include "HttpContext.h"
#include "../net/Buffer.h"
using namespace ssxrver;
using namespace ssxrver::net;

bool HttpContext::processRequestLine(const char *begin, const char *end)
{
    bool succeed = false;
    const char *start = begin;
    const char *space = std::find(start, end, ' ');      //根据协议格式，先查找空格所在位置
    if (space != end && request_.setMethod(start, space)) //解析请求方法
    {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end)
        {
            const char *question = std::find(start, space, '?');
            if (question != space)
            {
                request_.setPath(start, question);
                request_.setQuery(question, space);
            }
            else
            {
                request_.setPath(start, space); //解析path
            }
            start = space + 1;
            succeed = end - start == 8 && std::equal(start, end, "HTTP/1.1");
        }
    }
    return succeed;
}

bool HttpContext::parseRequest(Buffer *buf)
{
    bool ok = true;
    bool hasMore = true;
    while (hasMore) //相当与一个状态机
    {
        if (expectRequestLine()) //处于解析请求行状态
        {
            const char *crlf = buf->findCRLF(); //这些数据都保存到缓冲区当中，在缓冲区寻找\r\n，头部每一行都有一个\r\n
            if (crlf)
            {
                ok = processRequestLine(buf->peek(), crlf); //解析请求行
                if (ok)
                {
                    buf->retrieveUntil(crlf + 2);                   //将请求行从buf中取回，包括\r\n，所以要+2
                    receiveRequestLine();                  //httpcontext将状态改为kexpectheaders
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
        else if (expectHeaders()) //处于解析header的状态
        {
            const char *crlf = buf->findCRLF();
            if (crlf)
            {
                const char *colon = std::find(buf->peek(), crlf, ':'); //查找冒号所在位置
                if (colon != crlf)
                {
                    request().addHeader(buf->peek(), colon, crlf);
                }
                else
                {
                    // 空行，头解析完毕
                    receiveHeaders(); //httpcontext将状态改为kgotall
                    hasMore = !gotAll();
                }
                buf->retrieveUntil(crlf + 2); //将header从buf中取回，包括\r\n
            }
            else
            {
                hasMore = false;
            }
        }
        else if (expectBody()) //当前还暂时不支持带body，需要补充
        {
        }
    }
    return ok;
}
