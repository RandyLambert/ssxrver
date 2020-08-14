//
// Created by randylambert on 2020/8/11.
//

#include <boost/assert.hpp>
#include <cstdlib>
//#include <iostream>
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
    std::map<string,ssxrver::Logger::LogLevel> logMap{{"DEBUG",ssxrver::Logger::LogLevel::DEBUG},
                                                      {"INFO",ssxrver::Logger::LogLevel::INFO},
                                                      {"WARN",ssxrver::Logger::LogLevel::WARN},
                                                      {"ERROR",ssxrver::Logger::LogLevel::ERROR},
                                                      {"FATAL",ssxrver::Logger::LogLevel::FATAL}};

    std::unordered_set<string> blocksIp;
}

void asyncOutput(const char *msg, size_t len)
{
    g_asyncLog->append(msg, len);
}

template <class T>
T getTypeConversion(std::string_view value, const T &def = T()) //类型转换模板函数，可设置默认值
{
    try
    {
        return boost::lexical_cast<T>(value);
    }
    catch (...)
    {
        return def;
    }
}

void initConf()
{
    file::ReadSmallFile conf("./conf/ssxrver.json");
    ssize_t len = 0;
    [[maybe_unused]] int err = conf.readToBuffer(&len);
    BOOST_ASSERT_MSG(err >= 0,"读取conf文件失败,请在conf目录下按照ssxrver.json.example的格式创建ssxrver.json文件");
    ssxrver::util::confData = CJsonObject(conf.buffer());

    try{
        if(!(confData["log"]("ansync_started") == "on") && !(confData["log"]("ansync_started") == "off")){
            throw ;
        }
    } catch (...){
        confData["log"].Replace("ansync_started","off");
    }

    int flushSecond = getTypeConversion<int>(confData["log"]("flush_second"),3);
    confData["log"].Replace(
            ("flush_second"),
            flushSecond > 0 ? flushSecond : 3);

    int rollSize = getTypeConversion<int>(confData["log"]("roll_size"),65536);
    confData["log"].Replace(
            ("roll_size"),
            rollSize > 0 ? rollSize : 65536);

    int port = getTypeConversion<int>(confData("port"),4507);
    confData.Replace(
            ("port"),
            port > 0 ? port : 4507);

    int workerProcesses = getTypeConversion<int>(confData("worker_processes"),4);
    confData.Replace(
            ("worker_processes"),
            workerProcesses > 0 ? workerProcesses : 4);

    string ip;
    for(int i = 0;i < confData["blocks_ip"].GetArraySize();i++)
    {
        confData["blocks_ip"].Get(i,ip);
        blocksIp.insert(ip);
    }
}

void initAsyncLogThreadLog()
{
    std::string logName = confData["log"]("path") + confData["log"]("base_name");
    g_asyncLog.reset(new AsyncLogThread(logName,std::stoi(confData["log"]("flush_second")),
            std::stoi(confData["log"]("roll_size"))));
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
    if(confData["log"]("ansync_started") == "on")
        initAsyncLogThreadLog();
    ssxrver::Logger::setLogLevel(logMap[confData["log"]("level")]);

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