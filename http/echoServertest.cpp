#include <functional>
#include "echoServertest.h"

using namespace ssxrver;
using namespace ssxrver::net;
EchoServer::EchoServer(EventLoop* loop,const struct sockaddr_in listenAddr)
    : server_(loop,listenAddr)
{
    server_.setConnectionCallback(
                                  std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(
                               std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
}
void EchoServer::start()
{
    server_.start();
}
void EchoServer::onConnection(const TcpConnectionPtr& conn)
{
    conn->send("hello\n");
}
void EchoServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf)
{
    string msg(buf->retrieveAllAsString());
    if (msg == "exit\n")
    {
        Buffer a;
        conn->send(msg);
        /* conn->send("bye\n"); */
        conn->shutdown();
    }
    if (msg == "quit\n")
    {
        /* loop_->quit(); */
    }

    conn->send(msg);
    /* conn->send(msg); */
}
