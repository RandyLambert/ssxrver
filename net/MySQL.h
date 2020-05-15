#ifndef SSXERVER_NET_MYSQL_H
#define SSXERVER_NET_MYSQL_H
#include "../base/noncopyable.h"
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <mysql/mysql.h>
namespace ssxrver
{
namespace net
{

using std::string;
class CJsonObject;
class MySQL : noncopyable
{
public:
    ~MySQL();
    MySQL(const string& addr="127.0.0.1",const string& user="root",const string& password="123456",const string& dataBaseName="ttms",unsigned int port = 0,const char* unixSocket=NULL,unsigned long clientFlag = 0);
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

} // namespace net
} // namespace ssxrver
#endif
