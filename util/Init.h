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

extern CJsonObject confData;
extern std::unique_ptr<ssxrver::base::AsyncLogThread> g_asyncLog;
extern const char *http11;
extern const char *http10;
extern std::unordered_set<std::string> blocksIp;
extern int httpMaxBodySize;
extern int workerConnections;
void start();
}

#endif //SSXRVER_UTIL_INIT_H
