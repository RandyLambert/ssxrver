#include <string>
#include "MySQLsOps.h"
#include "MySQL.h"
#include "Logging.h"
using namespace ssxrver;
using namespace ssxrver::net;
string SQLs::sqlRegister(const string& str)
{
    LOG_INFO<<"sqlRegister";
    string query = str;
    return query;
}
string SQLs::sqlLogin(const string& str)
{
    LOG_INFO<<"sqlLogin";
    string query = str;
    return query;
}
