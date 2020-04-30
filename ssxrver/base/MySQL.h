#ifndef SSXERVER_NET_MYSQL_H
#define SSXERVER_NET_MYSQL_H
#include "noncopyable.h"
#include <string>
#include <map>
#include <functional>
#include <mysql/mysql.h>
namespace ssxrver
{
namespace net
{

using std::string;
class MySQL : noncopyable
{
    typedef std::function<string (const string&)> sqlCallBack;
public:
    MySQL();
    ~MySQL();
    bool mysqlInit(const string& addr="127.0.0.1",const string& user="root",const string& password="123456",const string& dataBaseName="ttms",unsigned int port = 0,const char* unixSocket=NULL,unsigned long clientFlag = 0);
    int returnMin()
    {
        return MIN;
    }
    int returnMid()
    {
        return MID;
    }
    int returnMax()
    {
        return MAX;
    }

    int useNoResultMap(int x,const string& query)
    {
        return queryNoResult(sqlMap[x](query));
    }

    int useHasResultMap(int x,const string& query,string& reback)
    {
        return queryHasResult(sqlMap[x](query),reback);
    }

private:

    enum
    {
        MIN,
        REGISTER,
        LOGIN,
        MID,
        MAX
    };
    int queryNoResult(const string& s);
    int queryHasResult(const string& s,string &result);
    std::map<int,sqlCallBack> sqlMap;
    MYSQL mysql_;
    MYSQL_RES *res_;
    MYSQL_ROW sqlrow_;
};

}
}
#endif

