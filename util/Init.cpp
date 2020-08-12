//
// Created by randylambert on 2020/8/11.
//

#include <boost/assert.hpp>
#include <cstdlib>
//#include <map>
#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "Init.h"
#include "Logging.h"
#include "EventLoop.h"
#include "../http/HttpServer.h"
#include "../base/File.h"

using namespace ssxrver::base;
using namespace ssxrver::net;
using namespace ssxrver::util;
using std::string;

namespace ssxrver::util
{
    CJsonObject confData;
    std::unique_ptr<AsyncLogThread> g_asyncLog;
    bool outPutFlag = false;
    const char *http11 = "HTTP/1.1";
    const char *http10 = "HTTP/1.0";
}

void asyncOutput(const char *msg, size_t len)
{
    g_asyncLog->append(msg, len);
}


void initConf()
{
    file::ReadSmallFile conf("./conf/ssxrver.json");
    ssize_t len = 0;
    [[maybe_unused]] int err = conf.readToBuffer(&len);
    BOOST_ASSERT_MSG(err >= 0,"读取conf文件失败,请在conf目录下按照ssxrver.json.example的格式创建ssxrver.json文件");
    ssxrver::util::confData = CJsonObject(conf.buffer());

    try{
        if(!confData["log"]("ansync_started").compare("on")  && !confData["log"]("ansync_started").compare("off")){
            throw ;
        }
    } catch (...){
        std::cout<<"test"<<std::endl;
        confData["log"].Replace("ansync_started","off");
    }

    try{
        if(std::stoi(confData["log"]("flush_second")) <= 0){
            throw ;
        }
    } catch (...){
        confData["log"].Replace("flush_second",3);
    }

    try{
        if(std::stoi(confData["log"]("roll_size")) <= 0){
            throw ;
        }
    } catch (...){
        confData["log"].Replace("row_size",65536);
    }

    try{
        if(std::stoi(confData("port")) <= 0){
            throw ;
        }
    } catch (...) {
        confData.Replace("port",4507);
    }

    try{
        if(std::stoi(confData("worker_processes")) <= 0){
            throw ;
        }
    } catch (...){
        confData.Replace("worker_processes",4);
    }
}

void initAsyncLogThreadLog()
{
    std::string logName = confData["log"]("path") + confData["log"]("base_name");
    g_asyncLog.reset(new AsyncLogThread(logName,std::stoi(confData["log"]("flush_second")),std::stoi(confData["log"]("roll_size"))));
    g_asyncLog->start();
    ssxrver::Logger::setOutput(asyncOutput);
}

void message(const HttpRequest &req, HttpResponse *resp)
{
//    if (util::outPutFlag)
//    {
//        const std::map<std::string, string> &headers = req.headers();
//        for (const auto &x : headers)
//        {
//            std::cout << x.first << " " << x.second << std::endl;
//        }
//         std::cout << req.body() << std::endl;
//    }

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

void ssxrver::util::start()
{

    initConf();
    if(confData("ansync_started").compare("on"))
        initAsyncLogThreadLog();

    struct sockaddr_in serv_addr{};
    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_port = htons(std::stoi(confData("port")));
    serv_addr.sin_family = AF_INET;
    if(inet_pton(AF_INET, confData("address").c_str(), &serv_addr.sin_addr) <= 0)
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    net::EventLoop loop;
    net::HttpServer server(&loop, serv_addr);
    server.setHttpCallback(message);
    server.setThreadNum(std::stoi(confData("worker_processes")));
    server.start();
    loop.loop();

}


