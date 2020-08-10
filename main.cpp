#include <iostream>
#include <string>
#include "http/HttpServer.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "net/EventLoop.h"
#include "net/MySQLsOps.h"
#include "net/CJsonObject.hpp"
#include "base/Logging.h"
#include "base/AsyncLogThread.h"
using namespace ssxrver;
using namespace ssxrver::net;
bool flag = true;
const char *http11 = "HTTP/1.1";
void message(const HttpRequest &req, HttpResponse *resp /*,  MySQLsOps *mysql*/)
{
    if (!flag)
    {
        const std::map<string, string> &headers = req.headers();
        for (const auto &x : headers)
        {
            std::cout << x.first << " " << x.second << std::endl;
        }
            /* std::cout << req.body() << std::endl; */
    }

    if (req.path() == "/")
    {
        resp->setStatusCode(HttpResponse::k2000k);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->setVersion(http11,8);
        resp->addHeader("Server", "ssxrver");
        resp->setBody("<html><head><title>This is title</title></head>"
                      "<body><h1>Hello World</h1></body></html>");
    }
}

ssxrver::base::AsyncLogThread *g_asyncLog = nullptr;
void asyncOutput(const char *msg, int len)
{
    g_asyncLog->append(msg, len);
}

int main(int argv, char *argc[])
{

    string logName = "./logs/ssxrver";
    ssxrver::base::AsyncLogThread log_(logName);
    log_.start();
    g_asyncLog = &log_;
    ssxrver::Logger::setOutput(asyncOutput);
    struct sockaddr_in serv_addr{};
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_port = htons(4507);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int threads = 4;
    EventLoop loop;
    HttpServer server(&loop, serv_addr);
    server.setHttpCallback(message);
    server.setThreadNum(threads);
    server.start();
    loop.loop();

    return 0;
}
