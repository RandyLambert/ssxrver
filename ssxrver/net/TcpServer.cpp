#include <cstdio>
#include "TcpServer.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "SocketOps.h"
#include "../base/Logging.h"
using namespace ssxrver;
using namespace ssxrver::net;

TcpServer::TcpServer(EventLoop *loop,
                     struct sockaddr_in listenAddr)
    :loop_(loop),
    threadPool_(new EventLoopThreadPool(loop)),
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback),
    nextConnId_(1)
{
}
