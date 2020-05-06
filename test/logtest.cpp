#include <iostream>
#include "../base/Logging.h"
#include "../base/LogStream.h"
using namespace ssxrver;
int main()
{
    LOG_INFO << "dasdsadsa" << 1 << "sdsdsa";
    LOG_TRACE << "DSDASDSA";
    LOG_DEBUG << "DSADSADASDSA";
    LOG_WARN << "DSADSADSADS";
    LOG_ERROR << "DDSADSADSA";
    LOG_SYSERR << "DASDSADAS";

    return 0;
}
