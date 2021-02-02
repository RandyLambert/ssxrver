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
void setCPUAffinity();
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
    const int& getHttpMaxBodySize() const { return  httpMaxBodySize_; }
    const std::unordered_set<std::string>& getBlocksIp() const { return  blocksIp_; }
    const CJsonObject& getConfData() const { return  confData_; }
    long getUsefulCpuNumber() {
        std::scoped_lock<std::mutex> lock(cpuMutex_);
        if(cpuUsedNumber_ >= cpuMaxNumber_) {
            return -1;
        } else {
            cpuUsedNumber_++;
            return cpuUsedNumber_ - 1;
        }
    }
private:
    explicit Init():outPutFlag_(false),
        http11_("HTTP/1.1"),
        http10_("HTTP/1.0"),
        logMap_({{"DEBUG",ssxrver::Logger::LogLevel::DEBUG},
                {"INFO",ssxrver::Logger::LogLevel::INFO},
                {"WARN",ssxrver::Logger::LogLevel::WARN}}),
        cpuUsedNumber_(0)
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
    long cpuUsedNumber_;
    long cpuMaxNumber_;
    std::mutex cpuMutex_;
};
}

#endif //SSXRVER_UTIL_INIT_H
