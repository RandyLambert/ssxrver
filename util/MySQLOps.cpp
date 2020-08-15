#include <string>
#include <vector>
#include "CJsonObject.hpp"
#include "MySQLOps.h"
using namespace ssxrver;
using namespace ssxrver::util;
using std::bind;
using namespace std::placeholders;
namespace 
{
using std::unique_ptr;
int sqlInsertUser(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson)
{
    return mysql->sqlInsert(cjson);
}
int sqlInsertMovie(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson)
{
    return mysql->sqlInsert(cjson);
}
int sqlInsertSeat(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson)
{
    return mysql->sqlInsert(cjson);
}
int sqlInsertSchedule(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson)
{
    return mysql->sqlInsert(cjson);
}
int sqlInsertStudio(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson)
{
    return mysql->sqlInsert(cjson);
}
int sqlInsertTicket(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson)
{
    return mysql->sqlInsert(cjson);
}
/******************************************/
int sqlUpdateUser(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson)
{
    return mysql->sqlUpdateWhere(cjson);
}
int sqlUpdateMovie(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson)
{
    return mysql->sqlUpdateWhere(cjson);
}
int sqlUpdateSeat(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson)
{
    return mysql->sqlUpdateWhere(cjson);
}
int sqlUpdateSchedule(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson)
{
    return mysql->sqlUpdateWhere(cjson);
}
int sqlUpdateStudio(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson)
{
    return mysql->sqlUpdateWhere(cjson);
}
int sqlUpdateTicket(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson)
{
    return mysql->sqlUpdateWhere(cjson);
}
/******************************************/
int sqlDeleteUser(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson)
{
    return mysql->sqlDeleteWhere(cjson);
}
int sqlDeleteMovie(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson)
{
    return mysql->sqlDeleteWhere(cjson);
}
int sqlDeleteSeat(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson)
{
    return mysql->sqlDeleteWhere(cjson);
}
int sqlDeleteSchedule(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson)
{
    return mysql->sqlDeleteWhere(cjson);
}

int sqlDeleteStudio(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson)
{
    return mysql->sqlDeleteWhere(cjson);
}
int sqlDeleteTicket(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson)
{
    return mysql->sqlDeleteWhere(cjson);
}
/*********************************************/
int sqlQueryUser(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson,CJsonObject& result)
{
    return mysql->sqlSelectWhere(cjson,result);
}
int sqlQueryMovie(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson,CJsonObject& result)
{
    return mysql->sqlSelectWhere(cjson,result);
}
int sqlQuerySeat(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson,CJsonObject& result)
{
    return mysql->sqlSelectWhere(cjson,result);
}
int sqlQuerySchedule(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson,CJsonObject& result)
{
    return mysql->sqlSelectWhere(cjson,result);
}
int sqlQueryStudio(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson,CJsonObject& result)
{
    return mysql->sqlSelectWhere(cjson,result);
}
int sqlQueryTicket(const unique_ptr<MySQL>& mysql,const CJsonObject& cjson,CJsonObject& result)
{
    return mysql->sqlSelectWhere(cjson,result);
}

}
MySQLOps::MySQLOps(const string& password,
                   const string& dataBaseName,
                   const string& addr,
                   const string& user,
                   unsigned int port,
                   const char* unixSocket,
                   unsigned long clientFlag)
    : mysql_(new MySQL(password,dataBaseName,addr,user,port,unixSocket,clientFlag))
{
    sqlNoResultlMap[INSERTUSER] = [](auto && arg1, auto && arg2) { return sqlInsertUser(arg1, arg2); };
    sqlNoResultlMap[INSERTMOVIE] = [](auto && arg1, auto && arg2) { return sqlInsertMovie(arg1, arg2); };
    sqlNoResultlMap[INSERTSEAT] = [](auto && arg1, auto && arg2) { return sqlInsertSeat(arg1, arg2); };
    sqlNoResultlMap[INSERTSCHEDULE] = [](auto && arg1, auto && arg2) { return sqlInsertSchedule(arg1, arg2); };
    sqlNoResultlMap[INSERTSTUDIO] = [](auto && arg1, auto && arg2) { return sqlInsertStudio(arg1, arg2); };
    sqlNoResultlMap[INSERTTICKET] = [](auto && arg1, auto && arg2) { return sqlInsertTicket(arg1, arg2); };
    /*********************************************************/
    sqlNoResultlMap[UPDATEUSER] = [](auto && arg1, auto && arg2) { return sqlUpdateUser(arg1, arg2); };
    sqlNoResultlMap[UPDATEMOVIE] = [](auto && arg1, auto && arg2) { return sqlUpdateMovie(arg1, arg2); };
    sqlNoResultlMap[UPDATESEAT] = [](auto && arg1, auto && arg2) { return sqlUpdateSeat(arg1, arg2); };
    sqlNoResultlMap[UPDATESCHEDULE] = [](auto && arg1, auto && arg2) { return sqlUpdateSchedule(arg1, arg2); };
    sqlNoResultlMap[UPDATESTUDIO] = [](auto && arg1, auto && arg2) { return sqlUpdateStudio(arg1, arg2); };
    sqlNoResultlMap[UPDATETICKET] = [](auto && arg1, auto && arg2) { return sqlUpdateTicket(arg1, arg2); };
    /*********************************************************/
    sqlNoResultlMap[DELETEUSER] = [](auto && arg1, auto && arg2) { return sqlDeleteUser(arg1, arg2); };
    sqlNoResultlMap[DELETEMOVIE] = [](auto && arg1, auto && arg2) { return sqlDeleteMovie(arg1, arg2); };
    sqlNoResultlMap[DELETESEAT] = [](auto && arg1, auto && arg2) { return sqlDeleteSeat(arg1, arg2); };
    sqlNoResultlMap[DELETESCHEDULE] = [](auto && arg1, auto && arg2) { return sqlDeleteSchedule(arg1, arg2); };
    sqlNoResultlMap[DELETESTUDIO] = [](auto && arg1, auto && arg2) { return sqlDeleteStudio(arg1, arg2); };
    sqlNoResultlMap[DELETETICKET] = [](auto && arg1, auto && arg2) { return sqlDeleteTicket(arg1, arg2); };
    /*********************************************************/
    sqlHasResultlMap[QUERYUSER] = [](auto && arg1, auto && arg2, auto && arg3) { return sqlQueryUser(arg1, arg2, arg3); };
    sqlHasResultlMap[QUERYMOVIE] = [](auto && arg1, auto && arg2, auto && arg3) { return sqlQueryMovie(arg1, arg2, arg3); };
    sqlHasResultlMap[QUERYSEAT] = [](auto && arg1, auto && arg2, auto && arg3) { return sqlQuerySeat(arg1, arg2, arg3); };
    sqlHasResultlMap[QUERYSCHEDULE] = [](auto && arg1, auto && arg2, auto && arg3) { return sqlQuerySchedule(arg1, arg2, arg3); };
    sqlHasResultlMap[QUERYSTUDIO] = [](auto && arg1, auto && arg2, auto && arg3) { return sqlQueryStudio(arg1, arg2, arg3); };
    sqlHasResultlMap[QUERYTICKET] = [](auto && arg1, auto && arg2, auto && arg3) { return sqlQueryTicket(arg1, arg2, arg3); };
}


