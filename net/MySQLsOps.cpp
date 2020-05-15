#include <string>
#include <iostream>
#include <vector>
#include <cstring>
#include "CJsonObject.hpp"
#include "MySQLsOps.h"
using namespace ssxrver;
using namespace ssxrver::net;
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

}
MySQLsOps::MySQLsOps(const string& addr,const string& user,const string& password,const string& dataBaseName,unsigned int port,const char* unixSocket,unsigned long clientFlag)
    : mysql_(new MySQL(addr,user,password,dataBaseName,port,unixSocket,clientFlag))
{
    sqlNoResultlMap[INSERTUSER] = bind(sqlInsertUser,_1,_2);
    sqlNoResultlMap[INSERTMOVIE] = bind(sqlInsertMovie,_1,_2);
    sqlNoResultlMap[INSERTSEAT] = bind(sqlInsertSeat,_1,_2);
    sqlNoResultlMap[INSERTSCHEDULE] = bind(sqlInsertSchedule,_1,_2);
    sqlNoResultlMap[INSERTSTUDIO] = bind(sqlInsertStudio,_1,_2);
    /*********************************************************/
    sqlNoResultlMap[UPDATEUSER] = bind(sqlUpdateUser,_1,_2);
    sqlNoResultlMap[UPDATEMOVIE] = bind(sqlUpdateMovie,_1,_2);
    sqlNoResultlMap[UPDATESEAT] = bind(sqlUpdateSeat,_1,_2);
    sqlNoResultlMap[UPDATESCHEDULE] = bind(sqlUpdateSchedule,_1,_2);
    sqlNoResultlMap[UPDATESTUDIO] = bind(sqlUpdateStudio,_1,_2);
    /*********************************************************/
    sqlNoResultlMap[DELETEUSER] = bind(sqlDeleteUser,_1,_2);
    sqlNoResultlMap[DELETEMOVIE] = bind(sqlDeleteMovie,_1,_2);
    sqlNoResultlMap[DELETESEAT] = bind(sqlDeleteSeat,_1,_2);
    sqlNoResultlMap[DELETESCHEDULE] = bind(sqlDeleteSchedule,_1,_2);
    sqlNoResultlMap[DELETESTUDIO] = bind(sqlDeleteStudio,_1,_2);
    /*********************************************************/
    sqlHasResultlMap[QUERYUSER] = bind(sqlQueryUser,_1,_2,_3);
    sqlHasResultlMap[QUERYMOVIE] = bind(sqlQueryMovie,_1,_2,_3);
    sqlHasResultlMap[QUERYSEAT] = bind(sqlQuerySeat,_1,_2,_3);
    sqlHasResultlMap[QUERYSCHEDULE] = bind(sqlQuerySchedule,_1,_2,_3);
    sqlHasResultlMap[QUERYSTUDIO] = bind(sqlQueryStudio,_1,_2,_3);
}

