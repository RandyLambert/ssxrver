#include <iostream>
#include <string>
#include "http/echoServertest.h"
#include "http/HttpServer.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "net/EventLoop.h"
#include "net/MySQL.h"
#include "base/Logging.h"
#include "base/AsyncLogThread.h"
using namespace ssxrver;
using namespace ssxrver::net;
bool flag = true;

void message(const HttpRequest &req, HttpResponse *resp /*,  MySQL *mysql*/)
{
    if (!flag)
    {
        int i = 0;
        const std::map<string, string> &headers = req.headers();
        for (const auto &x : headers)
        {
            i++;
            if (i == 1)
                std::cout << x.first << " " << x.second << std::endl;
        }
    }

    if (req.path() == "/")
    {
        resp->setStatusCode(HttpResponse::k2000k);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->addHeader("Server", "ssxrver");
        resp->setBody("<html><head><title>This is title</title></head>"
                      "<body><h1>Hello World</h1></body></html>");

        /* int x = 1; */
        /* string p = "INSERT INTO studio VALUES(NULL,'IMAX大厅',5,7,'IMAX影厅,3D电影');"; */
        /* if (x > mysql->returnMin() || x < mysql->returnMid()) */
        /*     mysql->useNoResultMap(x, p); */
        /* else if (x > mysql->returnMid() || x < mysql->returnMax()) */
        /* { */
        /*     string reback; */
        /*     mysql->useHasResultMap(x, "query", reback); */
        /* } */
    }
}

ssxrver::base::AsyncLogThread *g_asyncLog = NULL;
void asyncOutput(const char *msg, int len)
{
    g_asyncLog->append(msg, len);
}

int main(int argv, char *argc[])
{

    /* string logName = "ssxrver"; */
    /* ssxrver::base::AsyncLogThread log_(logName); */
    /* log_.start(); */
    /* g_asyncLog = &log_; */
    /* ssxrver::Logger::setOutput(asyncOutput); */

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_port = htons(4507);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int threads = 2;
    EventLoop loop;
    /* loop.setname("大循环"); */
    HttpServer server(&loop, serv_addr);
    /* EchoServer server(&loop, serv_addr); */
    server.setHttpCallback(message);
    server.setThreadNum(threads);
    server.start();
    loop.loop();

    return 0;
}
