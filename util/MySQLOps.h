#ifndef SSXERVER_UTIL_MYSQLSOPS_H
#define SSXERVER_UTIL_MYSQLSOPS_H
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <boost/noncopyable.hpp>
#include "MySQL.h"
namespace ssxrver::util
{

using std::string;
using std::vector;
class CJsonObject;
class MySQLOps : boost::noncopyable
{
    using sqlNoResultCallBack = std::function<int (const std::unique_ptr<MySQL>&,const CJsonObject&)>;
    using sqlHasResultCallBack = std::function<int (const std::unique_ptr<MySQL>&,const CJsonObject&,CJsonObject &)>;
public:
    explicit MySQLOps(const string& password,
            const string& dataBaseName,
            const string& addr="127.0.0.1",
            const string& user="root",
            unsigned int port = 0,
            const char* unixSocket= nullptr,
            unsigned long clientFlag = 0);
    enum 
    {
        MIN,
        INSERTUSER,
        INSERTMOVIE,
        INSERTSEAT,
        INSERTSCHEDULE,
        INSERTSTUDIO,
        INSERTTICKET,
        UPDATEUSER,
        UPDATEMOVIE,
        UPDATESEAT,
        UPDATESCHEDULE,
        UPDATESTUDIO,
        UPDATETICKET,
        DELETEUSER,
        DELETEMOVIE,
        DELETESEAT,
        DELETESCHEDULE,
        DELETESTUDIO,
        DELETETICKET,
        MID,
        QUERYUSER,
        QUERYMOVIE,
        QUERYSEAT,
        QUERYSCHEDULE,
        QUERYSTUDIO,
        QUERYTICKET,
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

#endif
