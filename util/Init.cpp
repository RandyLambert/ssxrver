//
// Created by randylambert on 2020/8/11.
//

#include <boost/assert.hpp>
#include <cstdlib>
#include <memory>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "Init.h"
#include "Logging.h"
#include "EventLoop.h"
#include "../http/HttpServer.h"

using namespace ssxrver::base;
using namespace ssxrver::net;
using namespace ssxrver::util;
using std::string;

namespace ssxrver::util
{
void setCPUAffinity() {
    LOG_INFO << "setCPUAffinity";
    long cpuNum = Init::getInstance().getUsefulCpuNumber();
    if(cpuNum != -1){
//        LOG_INFO << "setCPUAffinity";
        cpu_set_t mask; // CPU 核的集合
        CPU_ZERO(&mask); // 置空
        CPU_SET(cpuNum,&mask);
        if(pthread_setaffinity_np(pthread_self(), sizeof(mask),&mask) < 0) // 设置 thread CPU 亲和力
        {
            LOG_SYSERR << "could not set CPU affinity";
        }
    }
}

char favicon[] = {
#include "favicon.inc"
};
}

void asyncOutput(const char *msg, size_t len)
{
    Init::getInstance().getAsyncLog()->append(msg, len);
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

void Init::initAsyncLogThreadLog()
{
    std::string logName = confData_["log"]("path") + confData_["log"]("base_name");
    asyncLog_ = std::make_unique<AsyncLogThread>(logName,std::stoi(confData_["log"]("flush_second")),
            std::stoi(confData_["log"]("roll_size")));
    asyncLog_->start();
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
    if(req.path() == "/") {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/plain");
        resp->setVersion(Init::getInstance().getHttp11(),8);
        resp->addHeader("Server", "ssxrver");
        resp->setBody("Hello ssxrver!\n");
    }
    else if (req.path() == "/html") {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->setVersion(Init::getInstance().getHttp11(),8);
        resp->addHeader("Server", "ssxrver");
        resp->setBody("<html><head><title>Fast HttpServer</title></head>"
                      "<body><h1>Hello html</h1></body></html>");
    } else if(req.path() == "/index.html") {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/plain");
        resp->setVersion(Init::getInstance().getHttp11(),8);
        resp->addHeader("Server", "ssxrver");
        resp->setFile("./html/index.html");
    } else if(req.path() == "/favicon.ico") {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("image/png");
        resp->setVersion(Init::getInstance().getHttp11(),8);
        resp->setBody(string(favicon,sizeof(favicon)));
    } else {
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setVersion(Init::getInstance().getHttp11(),8);
        resp->setCloseConnection(true);
    }
}

void Init::start() {
    initConf();
    if(confData_["log"]("ansync_started") == "on")
        initAsyncLogThreadLog();
    ssxrver::Logger::setLogLevel(logMap_[confData_["log"]("level")]);

    struct sockaddr_in serv_addr{};
    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_port = htons(std::stoi(confData_("port")));
    serv_addr.sin_family = AF_INET;
    if(inet_pton(AF_INET, confData_("address").c_str(), &serv_addr.sin_addr) <= 0)
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    net::EventLoop loop;
    net::HttpServer server(&loop, serv_addr);
    server.setHttpCallback(message);
    server.setThreadNum(std::stoi(confData_("worker_processes")));
    server.start();
    loop.loop();
}

void Init::initConf() {

    file::ReadSmallFile conf("./conf/ssxrver.json");
    ssize_t len = 0;
    [[maybe_unused]] int err = conf.readToBuffer(&len);
    BOOST_ASSERT_MSG(err >= 0,"读取conf文件失败,请在conf目录下按照ssxrver.json.example的格式创建ssxrver.json文件");
    confData_ = CJsonObject(conf.buffer());

    try{
        if(!(confData_["log"]("ansync_started") == "on") && !(confData_["log"]("ansync_started") == "off")){
            throw ;
        }
    } catch (...){
        confData_["log"].Replace("ansync_started","off");
    }

    if(confData_("cpu_affinity") == "on") {
        cpuMaxNumber_ = sysconf(_SC_NPROCESSORS_CONF); //获取 CPU 核数
        setCPUAffinity();
    }

    int flushSecond = getTypeConversion<int>(confData_["log"]("flush_second"),3);
    confData_["log"].Replace(
            ("flush_second"),
            flushSecond > 0 ? flushSecond : 3);

    int rollSize = getTypeConversion<int>(confData_["log"]("roll_size"),67108864);
    confData_["log"].Replace(
            ("roll_size"),
            rollSize > 0 ? rollSize : 67108864);

    int port = getTypeConversion<int>(confData_("port"),4507);
    confData_.Replace(
            ("port"),
            port > 0 ? port : 4507);

    workerConnections_ = getTypeConversion<int>(confData_("worker_connections"),-1);

    int max_body_size = getTypeConversion<int>(confData_["http"]("max_body_size"),67108864);
    confData_["http"].Replace(
            ("max_body_size"),
            max_body_size > 0 ? max_body_size : 67108864);
    httpMaxBodySize_ = max_body_size;

    int workerProcesses = getTypeConversion<int>(confData_("worker_processes"),4);
    confData_.Replace(
            ("worker_processes"),
            workerProcesses > 0 ? workerProcesses : 4);

    string ip;
    for(int i = 0;i < confData_["blocks_ip"].GetArraySize();i++)
    {
        confData_["blocks_ip"].Get(i,ip);
        blocksIp_.insert(ip);
    }
}