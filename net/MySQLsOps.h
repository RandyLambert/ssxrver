#ifndef SSXERVER_NET_MYSQLSOPS_H
#define SSXERVER_NET_MYSQLSOPS_H
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include "MySQL.h"
#include "../base/noncopyable.h"
namespace ssxrver
{
namespace net
{

using std::string;
using std::vector;
class CJsonObject;
class MySQLsOps : noncopyable 
{
    typedef std::function<int (const std::unique_ptr<MySQL>&,const CJsonObject&)> sqlNoResultCallBack;
    typedef std::function<int (const std::unique_ptr<MySQL>&,const CJsonObject&,CJsonObject &)> sqlHasResultCallBack;
public:
    MySQLsOps(const string& addr="127.0.0.1",const string& user="root",const string& password="123456",const string& dataBaseName="ttms",unsigned int port = 0,const char* unixSocket=NULL,unsigned long clientFlag = 0);
    enum 
    {
        MIN,
        INSERTUSER,
        INSERTMOVIE,
        INSERTSEAT,
        INSERTSCHEDULE,
        INSERTSTUDIO,
        UPDATEUSER,
        UPDATEMOVIE,
        UPDATESEAT,
        UPDATESCHEDULE,
        UPDATESTUDIO,
        DELETEUSER,
        DELETEMOVIE,
        DELETESEAT,
        DELETESCHEDULE,
        DELETESTUDIO,
        MID,
        QUERYUSER,
        QUERYMOVIE,
        QUERYSEAT,
        QUERYSCHEDULE,
        QUERYSTUDIO,
        MAX
    };

    int queryNoResult(int x,const CJsonObject& query)
    {
        return sqlNoResultlMap[x](mysql_,query);
    }

    int queryHasResult(int x,const CJsonObject& query,CJsonObject& reback)
    {
        return sqlHasResultlMap[x](mysql_,query,reback);
    }

private:
    std::unique_ptr<MySQL> mysql_;
    std::map<int,sqlNoResultCallBack> sqlNoResultlMap;
    std::map<int,sqlHasResultCallBack> sqlHasResultlMap;

};


}
}

#endif
