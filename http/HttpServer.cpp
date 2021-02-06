#include <functional>
#include <unistd.h>
#include <chrono>
#include "HttpServer.h"
#include "HttpParser.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Init.h"
using namespace ssxrver;
using namespace ssxrver::net;
using namespace std::chrono_literals;
namespace ssxrver::net::detail
{

void defaultHttpCallback(const HttpRequest &, HttpResponse *resp)
{
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}
void threadInitCallback(EventLoop*)
{
    ssxrver::util::setCPUAffinity();
}
} // namespace ssxrver::net::detail


HttpServer::HttpServer(EventLoop *loop,
                       const struct sockaddr_in listenAddr,
                       int taskProcesses)
    : threadPoolNumber_(taskProcesses),
      server_(loop, listenAddr),
      httpCallback_(detail::defaultHttpCallback)
{
    server_.setConnectionCallback(std::bind(&HttpServer::onConnection,this,std::placeholders::_1));
    server_.setMessageCallback(bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    if(ssxrver::util::Init::getInstance().getCpuAffinity())
        server_.setThreadInitCallback(bind(ssxrver::net::detail::threadInitCallback,std::placeholders::_1));
}

void HttpServer::start()
{
    server_.start();
    if(threadPoolNumber_ > 0){
        threadPool_.start(threadPoolNumber_);
    }
}

void HttpServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buf)
{
    HttpRequestParser *parser = conn->getMutableContext().get(); //获取的是可以改变的
    parser->execute(buf);
    if (parser->hasError()) //获取请求包，更好的做法是让 parseRequest 作为 httpContext 的成员函数
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
            }
            buf->retrieveUntil(end);
        }
        else
        {
            onRequest(conn, parser->getRequest()); //连接和请求对象传过来
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
    if(response.hasFile())
    {
        conn->setSendFile(response.getFile());
    }
    conn->send(&buf);               //将缓冲区发送到客户端
    if (response.closeConnection()) //如果需要关闭，短连接
        conn->shutdown();
    else {
        conn->getContext()->reset();                       //本次请求处理完毕，重置httpContext，适用于长连接 */
        if(conn->isSendFile())
            conn->sendFileReset();
    }
}

void HttpServer::onConnection(const TcpConnectionPtr &conn) {
//    server_.getLoop()->runAfter(2s,[]{ LOG_WARN<<"runAfter1s"; });
//    server_.getLoop()->runAfter(3s,[]{ LOG_WARN<<"runAfter2s"; });
//    server_.getLoop()->runEvery(3s,[]{ LOG_WARN<<"runEvery2s"; });
//    threadPool_.addTask([]{LOG_WARN<<"ThreadPool Task";});
}
