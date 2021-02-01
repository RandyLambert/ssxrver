//
// Created by randylambert on 2020/8/11.
//

#ifndef SSXRVER_UTIL_INIT_H
#define SSXRVER_UTIL_INIT_H

#include "CJsonObject.hpp"
#include "../base/AsyncLogThread.h"
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include <unordered_set>

namespace ssxrver::util
{

//extern CJsonObject confData;
//extern std::unique_ptr<ssxrver::base::AsyncLogThread> g_asyncLog;
//extern const char *http11;
//extern const char *http10;
//extern std::unordered_set<std::string> blocksIp;
//extern int httpMaxBodySize;
//extern int workerConnections;
//void start();

class Init : boost::noncopyable
{
public:
    static Init& getInstance(){
        static Init instance;
        return instance;
    }
    void start();
    const char* getHttp11() const { return http11_;}
    const char* getHttp10() const {return http10_; }
    const std::unique_ptr<ssxrver::base::AsyncLogThread>& getAsyncLog() const { return asyncLog_;}
    const int& getWorkerConnections() const { return workerConnections_;}
    const int& getHttpMaxBodySize() const {return  httpMaxBodySize_; }
    const std::unordered_set<std::string>& getBlocksIp() const {return  blocksIp_; }
    const CJsonObject& getConfData() const {return  confData_; }
private:
    Init():outPutFlag_(false),
        http11_("HTTP/1.1"),
        http10_("HTTP/1.0"),
        logMap_({{"DEBUG",ssxrver::Logger::LogLevel::DEBUG},
                {"INFO",ssxrver::Logger::LogLevel::INFO},
                {"WARN",ssxrver::Logger::LogLevel::WARN}})
    {}
    ~Init() = default;
    void initConf();
    void initAsyncLogThreadLog();
    CJsonObject confData_;
    std::unique_ptr<ssxrver::base::AsyncLogThread> asyncLog_;
    bool outPutFlag_;
    const char *http11_;
    const char *http10_;
    std::map<std::string,ssxrver::Logger::LogLevel> logMap_;
    std::unordered_set<std::string> blocksIp_;
    int httpMaxBodySize_;
    int workerConnections_;
    int cpuNumber;
    int cpuNow;
};
}

#endif //SSXRVER_UTIL_INIT_H
