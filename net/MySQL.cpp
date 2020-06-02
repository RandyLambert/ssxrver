#include "MySQL.h"
#include "MySQLsOps.h"
#include "CJsonObject.hpp"
#include "../base/Logging.h"
using namespace ssxrver;
using namespace ssxrver::net;

MySQL::MySQL(const string &addr, const string &user, const string &password, const string &dataBaseName, unsigned int port, const char *unixSocket, unsigned long clientFlag)
    : res_(NULL),
      sqlrow_(0)
{
    res_ = nullptr;
    sqlrow_ = 0;
    res_ = 0;
    if (NULL == mysql_init(&mysql_))
    {
        LOG_SYSFATAL << mysql_error(&mysql_);
    }

    //初始化一个句柄;
    //初始化数据库
    if (mysql_library_init(0, NULL, NULL) != 0)
    {
        LOG_SYSFATAL << mysql_error(&mysql_);
    }

    if (mysql_real_connect(&mysql_, addr.c_str(), user.c_str(), password.c_str(), dataBaseName.c_str(), port, unixSocket, clientFlag) == NULL)
    {
        LOG_SYSFATAL << mysql_error(&mysql_);
    }

    //连接数据库重要的一步:
    //设置中文字符集
    if (mysql_set_character_set(&mysql_, "utf8") < 0)
    {
        LOG_SYSFATAL << mysql_error(&mysql_);
    }
    LOG_INFO << "mysql init";
}

MySQL::~MySQL()
{
    mysql_close(&mysql_);
    mysql_library_end();
    LOG_DEBUG << "mysql close";
}

int MySQL::queryTableColName(const string &tableName, CJsonObject &result)
{
    string s = "SELECT column_name FROM information_schema.columns WHERE table_schema='ttms' AND table_name='" + tableName + "';";
    result.AddEmptySubArray("what");
    if (mysql_query(&mysql_, s.c_str()) != 0)
    {
        LOG_DEBUG << "queryTableColName " << mysql_error(&mysql_);
        return -1;
    }

    if (!(res_ = mysql_use_result(&mysql_)))
    {
        return -1;
    }

    int count = mysql_num_fields(res_);
    while ((sqlrow_ = mysql_fetch_row(res_)))
    {
        for (int j = 0; j < count; j++)
        {
            result["what"].Add(sqlrow_[j]);
        }
    }
    mysql_free_result(res_);
    LOG_DEBUG << "queryColName success";
    return 1;
}

int MySQL::queryNoResult(const string &s)
{
    if (mysql_query(&mysql_, s.c_str()) != 0)
    {
        LOG_DEBUG << "queryNoResult " << mysql_error(&mysql_);
        return -1;
    }
    else
    {
        LOG_DEBUG << "queryNoResult success";
        return static_cast<int>(mysql_affected_rows(&mysql_));
    }
}

int MySQL::queryHasResult(const CJsonObject &s, CJsonObject &result)
{
    result.AddEmptySubArray("data");
    if (mysql_query(&mysql_, s("queryStr").c_str()) != 0)
    {
        LOG_DEBUG << "queryHasResult " << mysql_error(&mysql_);
        return -1;
    }

    if (!(res_ = mysql_use_result(&mysql_)))
    {
        return -1;
    }

    int count = mysql_num_fields(res_);
    CJsonObject temp;
    CJsonObject sRef = const_cast<CJsonObject &>(s);
    while ((sqlrow_ = mysql_fetch_row(res_)))
    {
        temp.Clear();
        for (int j = 0; j < count; j++)
        {
            temp.Add(sRef["what"](j), sqlrow_[j]);
        }
        result["data"].Add(temp);
    }
    mysql_free_result(res_);
    LOG_DEBUG << "queryHasResult success";
    return 1;
}

int MySQL::sqlInsert(const CJsonObject &cjson)
{
    string queryStr("INSERT INTO " + cjson("tableName") + "(");
    string keyStr;
    string valueStrs("VALUES");
    bool flag = false;
    CJsonObject &cjsonRef = const_cast<CJsonObject &>(cjson);
    for (int i = 0; i < cjsonRef["data"].GetArraySize(); i++)
    {
        if (flag == false)
        {
            flag = true;
            valueStrs += "(";
            while (cjsonRef["data"][i].GetKey(keyStr))
            {
                queryStr += keyStr + ",";
                valueStrs += cjsonRef["data"][i](keyStr) + ",";
            }
            queryStr.back() = ')';
            valueStrs.back() = ')';
            queryStr += valueStrs + ",";
        }
        else
        {
            valueStrs.clear();
            valueStrs += "(";
            while (cjsonRef["data"][i].GetKey(keyStr))
            {
                valueStrs += cjsonRef["data"][i](keyStr) + ",";
            }
            valueStrs.back() = ')';
            queryStr += valueStrs + ",";
        }
    }
    queryStr.back() = ';';
    LOG_DEBUG << queryStr;
    return queryNoResult(queryStr);
}
int MySQL::sqlSelectWhere(const CJsonObject &cjson, CJsonObject &result)
{

    string queryStr("SELECT ");
    string keyStr;
    CJsonObject &cjsonRef = const_cast<CJsonObject &>(cjson);
    for (int i = 0; i < cjsonRef["what"].GetArraySize(); i++)
    {
        queryStr += cjsonRef["what"](i) + ',';
    }

    queryStr.back() = ' ';
    queryStr += "FROM " + cjson("tableName");
    if (!cjson("op").empty())
    {
        queryStr += " WHERE ";
        int i = 0;
        while (cjsonRef["data"].GetKey(keyStr))
        {
            if (i == 0)
            {
                queryStr += keyStr + cjsonRef["op"](i);
                queryStr += cjsonRef["data"](keyStr);
                i++;
            }
            else
            {
                queryStr += " AND " + keyStr + cjsonRef["op"](i);
                queryStr += cjsonRef["data"](keyStr);
                i++;
            }
        }
    }

    if (!cjsonRef("limit").empty())
    {
        queryStr += " LIMIT " + cjsonRef("limit");
    }
    queryStr += ';';

    CJsonObject query;
    if (cjsonRef["what"](0) == "*")
    {
        queryTableColName(cjson("tableName"), query);
    }
    else
    {
        query.AddEmptySubArray("what");
        query.Replace("what", cjsonRef["what"]);
    }
    query.Add("queryStr", queryStr);

    LOG_DEBUG << "query" << query.ToFormattedString();
    return queryHasResult(query, result);
}

int MySQL::sqlDeleteWhere(const CJsonObject &cjson)
{

    string queryStr("DELETE FROM " + cjson("tableName") + " WHERE ");
    string keyStr;
    CJsonObject &cjsonRef = const_cast<CJsonObject &>(cjson);
    int i = 0;
    while (cjsonRef["data"].GetKey(keyStr))
    {
        if (i == 0)
        {
            queryStr += keyStr + cjsonRef["op"](i);
            queryStr += cjsonRef["data"](keyStr);
            i++;
        }
        else
        {
            queryStr += " AND " + keyStr + cjsonRef["op"](i);
            queryStr += cjsonRef["data"](keyStr);
            i++;
        }
    }
    queryStr += ';';
    LOG_DEBUG << queryStr;
    return queryNoResult(queryStr);
}

int MySQL::sqlUpdateWhere(const CJsonObject &cjson)
{
    string queryStr("UPDATE " + cjson("tableName") + " SET ");
    string keyStr;
    CJsonObject &cjsonRef = const_cast<CJsonObject &>(cjson);
    while (cjsonRef["set"].GetKey(keyStr))
    {
        queryStr += keyStr + "=";
        queryStr += cjsonRef["set"](keyStr) + ",";
    }
    queryStr.back() = ' ';
    queryStr += "WHERE ";

    int i = 0;
    while (cjsonRef["data"].GetKey(keyStr))
    {
        if (i == 0)
        {
            queryStr += keyStr + cjsonRef["op"](i);
            queryStr += cjsonRef["data"](keyStr);
            i++;
        }
        else
        {
            queryStr += " AND " + keyStr + cjsonRef["op"](i);
            queryStr += cjsonRef["data"](keyStr);
            i++;
        }
    }
    queryStr += ';';

    LOG_DEBUG << queryStr;
    return queryNoResult(queryStr);
}
