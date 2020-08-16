#include <functional>
#include "HttpServer.h"
#include "HttpParser.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
using namespace ssxrver;
using namespace ssxrver::net;

namespace ssxrver::net::detail
{

void defaultHttpCallback(const HttpRequest &, HttpResponse *resp)
{
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}
} // namespace ssxrver::net::detail


HttpServer::HttpServer(EventLoop *loop,
                       const struct sockaddr_in listenAddr)
    : server_(loop, listenAddr),
      httpCallback_(detail::defaultHttpCallback)
{
    server_.setMessageCallback(bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
}

void HttpServer::start()
{
    server_.start();
}

void HttpServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buf)
{
    HttpRequestParser *parser = conn->getMutableContext().get(); //获取的是可以改变的
    parser->execute(buf);
    if (parser->hasError()) //获取请求包，更好的做法是让parserequest作为httpcontext的成员函数
    {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n"); //请求失败
        conn->shutdown();
    }
    //请求消息解析完毕
    if (parser->isFinished() == 1)
    {
        size_t length = parser->getContentLength();
        if (length > 0)
        {
            size_t len = length - parser->getRequest().body().size();
            const char *end = buf->peek() + (buf->readableBytes() < len ? buf->readableBytes() : len);
            parser->getRequest().addBody(buf->peek(), end);
            if (parser->getRequest().body().size() == length)
            {
                onRequest(conn, parser->getRequest()); //连接和请求对象传过来
                parser->reset();                       //本次请求处理完毕，重置httpcontext，适用于长连接 */
            }
            buf->retrieveUntil(end);
        }
        else
        {
            onRequest(conn, parser->getRequest()); //连接和请求对象传过来
            parser->reset();                       //本次请求处理完毕，重置httpcontext，适用于长连接 */
        }
    }
}

void HttpServer::onRequest(const TcpConnectionPtr &conn, const HttpRequest &req)
{
    const string &connection = req.getHeader<string>("Connection");
    HttpResponse response;
    response.setClose(connection);
    httpCallback_(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);  //将对象转化为一个字符串到buf中
    conn->send(&buf);               //将缓冲区发送到客户端
    if (response.closeConnection()) //如果需要关闭，短连接
        conn->shutdown();
}

void HttpServer::onConnection(const TcpConnectionPtr &conn) {

}
