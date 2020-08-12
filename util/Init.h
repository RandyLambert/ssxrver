//
// Created by randylambert on 2020/8/11.
//

#ifndef SSXRVER_UTIL_INIT_H
#define SSXRVER_UTIL_INIT_H

#include "../net/CJsonObject.hpp"
#include "../base/AsyncLogThread.h"
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"

namespace ssxrver::util
{

extern ssxrver::net::CJsonObject confData;
extern std::unique_ptr<ssxrver::base::AsyncLogThread> g_asyncLog;
extern const char *http11;
extern const char *http10;

void start();
}

#endif //SSXRVER_UTIL_INIT_H
