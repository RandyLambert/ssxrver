#ifndef SSXERVER_UTIL_MYSQL_H
#define SSXERVER_UTIL_MYSQL_H
#include <boost/noncopyable.hpp>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <mysql/mysql.h>
namespace ssxrver::util
{

using std::string;
class CJsonObject;
class MySQL : boost::noncopyable
{
public:
    ~MySQL();
    explicit MySQL(const string& password,
            const string& dataBaseName,
            const string& addr="127.0.0.1",
            const string& user="root",
            unsigned int port = 0,
            const char* unixSocket= nullptr ,
            unsigned long clientFlag = 0);
    int sqlSelectWhere(const CJsonObject& cjson,CJsonObject& result);
    int sqlDeleteWhere(const CJsonObject& cjson);
    int sqlUpdateWhere(const CJsonObject& cjson);
    int sqlInsert(const CJsonObject& cjson);
    int queryNoResult(const string& s);
    int queryHasResult(const CJsonObject& s,CJsonObject &result);
private:
    int queryTableColName(const string& s,CJsonObject& result);

    MYSQL mysql_;
    MYSQL_RES *res_;
    MYSQL_ROW sqlrow_;
};

} // namespace ssxrver::net
#endif
