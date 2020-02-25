#include <iostream>
#include <string>
#include "base/Logging.h"
#include "net/TcpServer.h"
#include "http/HttpServer.h"
using namespace ssxrver;
using namespace ssxrver::net;
bool flag = false;

void message(const HttpRequest &req,HttpResponse *resp)
{
    std::cout<<"have a message"<<std::endl;
    if(!flag)
    {
        const std::map<string,string>& headers = req.headers();
        for(const auto &x : headers)
            std::cout<<x.first<<" "x.second<<std::endl;
    }

    if(req.path() == "/")
    {
        resp->setStatusCode(HttpResponse::k200k);
        resp->setStausMessage("OK");
        resp->setContentType("text/html");
        resp->addHeader("Server","ssxrver");
        resp->setBody("hello wourd");
    }
}
int main(int argv,char *argc[])
{

    struct sockaddr_in serv_addr;
    bzero(&serv_addr,sizeof(serv_addr));
    serv_addr.sin_port = htons(4507);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int threads = 2;
    EventLoop loop;
    HttpServer server(&loop,serv_addr);
    server.setHttpCallback(message);
    server.setThreadNum(threads);
    server.start();
    loop.loop();

    return 0;
}

