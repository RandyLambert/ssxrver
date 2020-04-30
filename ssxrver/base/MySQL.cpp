#include "MySQL.h"
#include "MySQLsOps.h"
#include "Logging.h"
using namespace ssxrver;
using namespace ssxrver::net;
using namespace std::placeholders;

MySQL::MySQL():res_(NULL),sqlrow_(0)
{
}

MySQL::~MySQL()
{
    mysql_close(&mysql_);
    mysql_library_end();
    LOG_INFO<<"mysql close";
}

int MySQL::queryNoResult(const string& s)
{
    if(mysql_query(&mysql_,s.c_str()) != 0){
        LOG_INFO <<"queryNoResult "<< mysql_error(&mysql_);
        return -1;
    }
    else
    {
        LOG_INFO <<"queryNoResult success";
        return static_cast<int>(mysql_affected_rows(&mysql_));
    }
}

int MySQL::queryHasResult(const string& s,string& result)
{
    if(mysql_query(&mysql_,s.c_str()) != 0){
        LOG_INFO <<"queryHasResult "<< mysql_error(&mysql_);
        return -1;
    }

    if(!(res_ = mysql_use_result(&mysql_)))
    {
        return -1;
    }

    int count = mysql_num_fields(res_);
    while((sqlrow_ = mysql_fetch_row(res_)))
    {
        for(int j = 0;j < count;j++)
        {
            result+=sqlrow_[j];
            result+="\r\t";
        }
        result+="\r\t";
    }
    mysql_free_result(res_);
    LOG_INFO <<"queryHasResult success";
    return 1;
}

bool MySQL::mysqlInit(const string& addr,const string& user,const string& password,const string& dataBaseName,unsigned int port,const char* unixSocket,unsigned long clientFlag)
{
    res_ = nullptr;
    sqlrow_ = 0;
    res_ = 0;
    if(NULL == mysql_init(&mysql_))
    {
        LOG_FATAL << mysql_error(&mysql_);
        return false;
    }

    //初始化一个句柄;
    //初始化数据库
    if(mysql_library_init(0,NULL,NULL) != 0)
    {
        LOG_FATAL << mysql_error(&mysql_);
        return false;
    }

    if(mysql_real_connect(&mysql_,addr.c_str(),user.c_str(),password.c_str(),dataBaseName.c_str(),port,unixSocket,clientFlag)==NULL)
    {
        LOG_FATAL<< mysql_error(&mysql_);
        return false;
    }

    //连接数据库重要的一步:
    //设置中文字符集
    if(mysql_set_character_set(&mysql_,"utf8") < 0)
    {
        LOG_FATAL<< mysql_error(&mysql_);
        return false;
    }

    /* int x = 1; */
    /* if(x > mysqltest.returnMin() || x < mysqltest.returnMid()) */
    /*    mysqltest.useNoResultMap(x,p); */
    /* else if(x > mysqltest.returnMid() || x < mysqltest.returnMax()) */
    /* { */
    /*     string reback; */
    /*     mysqltest.useHasResultMap(x,"query",reback); */
    sqlMap[REGISTER] = bind(SQLs::sqlRegister,_1);
    sqlMap[LOGIN] = bind(SQLs::sqlLogin,_1);

    return true;
}
