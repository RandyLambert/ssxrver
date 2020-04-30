#include <iostream>
#include <string>
#include "http/HttpServer.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "net/EventLoop.h"
#include "base/Logging.h"
#include "base/MySQL.h"
using namespace ssxrver;
using namespace ssxrver::net;
bool flag = false;

void message(const HttpRequest &req, HttpResponse *resp, MySQL *mysql)
{
    std::cout << "have a message" << std::endl;
    if (!flag)
    {
        const std::map<string, string> &headers = req.headers();
        for (const auto &x : headers)
            std::cout << x.first << " " << x.second << std::endl;
    }

    if (req.path() == "/")
    {
        resp->setStatusCode(HttpResponse::k2000k);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->addHeader("Server", "ssxrver");
        resp->setBody("<html><head><title>This is title</title></head>"
                      "<body><h1>Hello World</h1></body></html>");

        int x = 1;
        string p = "INSERT INTO studio VALUES(NULL,'IMAX大厅',5,7,'IMAX影厅,3D电影');";
        if (x > mysql->returnMin() || x < mysql->returnMid())
            mysql->useNoResultMap(x, p);
        else if (x > mysql->returnMid() || x < mysql->returnMax())
        {
            string reback;
            mysql->useHasResultMap(x, "query", reback);
        }
    }
}

int main(int argv, char *argc[])
{

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_port = htons(4507);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int threads = 2;
    EventLoop loop;
    HttpServer server(&loop, serv_addr);
    server.setHttpCallback(message);
    server.setThreadNum(threads);
    server.start();
    loop.loop();

    return 0;
}
