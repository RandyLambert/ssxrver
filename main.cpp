#include <iostream>
#include <string>
#include "http/echoServertest.h"
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
bool flag = false;

void message(const HttpRequest &req, HttpResponse *resp ,  MySQLsOps *mysql)
{
    if (!flag)
    {
        const std::map<string, string> &headers = req.headers();
        for (const auto &x : headers)
        {
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


        CJsonObject obj1;
        /****************************登录*/
        obj1.AddEmptySubArray("what");
        obj1["what"].Add("*");
        /* obj1["what"].Add("userName"); */
        /* obj1["what"].Add("PassWord"); */
        /* obj1["what"].Add("sex"); */
        obj1.AddEmptySubArray("op");
        obj1["op"].Add("=");
        obj1["op"].Add("=");
        obj1.Add("tableName","user");
        obj1.AddEmptySubObject("data");
        obj1["data"].Add("userName","'管理员'");
        obj1["data"].Add("passWord","'123456'");
        std::cout<<obj1.ToFormattedString()<<std::endl;
        int x = MySQLsOps::QUERYUSER;
        CJsonObject reback;
        if (x > MySQLsOps::MIN && x < MySQLsOps::MID)
        {
            if(mysql->queryNoResult(x, obj1) == -1)
            {
                reback.Add("state",400);
                reback.Add("message","传递信息");
            }
            else
            {
                reback.Add("state",200);
                reback.Add("message","传递信息");
            }
        }
        else if (x > MySQLsOps::MID && x < MySQLsOps::MAX)
        {
            if(mysql->queryHasResult(x, obj1, reback) == -1)
            {
                reback.Add("state",400);
                reback.Add("message","传递信息");
            }
            else
            {
                reback.Add("state",200);
                reback.Add("message","传递信息");
            }
        }
        std::cout<<reback.ToFormattedString()<<std::endl;

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
