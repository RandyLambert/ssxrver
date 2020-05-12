#ifndef SSXRVER_TEST_ECHOSERVERTEST_H
#define SSXRVER_TEST_ECHOSERVERTEST_H
#include "../net/TcpServer.h"
#include "../base/Logging.h"
#include "../net/EventLoop.h"
namespace ssxrver
{
namespace net
{

class EchoServer
{
 public:
  EchoServer(EventLoop* loop,const struct sockaddr_in listenAddr);
  void start();
  void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

 private:
  void onConnection(const TcpConnectionPtr& conn);
  void onMessage(const TcpConnectionPtr& conn, Buffer* buf);

  EventLoop* loop_;
  TcpServer server_;
};

}
}
#endif
