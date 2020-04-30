#ifndef SSXRVER_HTTP_HTTPREQUEST_H
#define SSXRVER_HTTP_HTTPREQUEST_H
#include <map>
#include <assert.h>
#include <cstdio>
#include <string>

namespace ssxrver
{ //http请求类封装
namespace net
{
using std::string;
class HttpRequest
{
public:
    enum Method
    {
        kInvalid, //请求方法
        kGet,
        kHead,
    };
    HttpRequest()
        : method_(kInvalid)
    {
    }

    bool setMethod(const char *start, const char *end)
    {
        assert(method_ == kInvalid);
        std::string m(start, end); //根据这个字符串来判断
        if (m == "GET")
        {
            method_ = kGet;
        }
        else if (m == "HEAD")
        {
            method_ = kHead;
        }
        else
        {
            method_ = kInvalid;
        }
        return method_ != kInvalid; //看是否请求成功
    }

    Method method() const { return method_; }
    const char *methodString() const //请求方法转换为字符串
    {
        const char *result = "UNKNOWN";
        switch (method_)
        {
        case kGet:
            result = "GET";
            break;
        case kHead:
            result = "HEAD";
            break;
        default:
            break;
        }
        return result;
    }

    void setPath(const char *start, const char *end) { path_.assign(start, end); } //设置路径
    const string &path() const { return path_; }
    void setQuery(const char *start, const char *end) { query_.assign(start, end); }
    const string query() const { return query_; }
    void addHeader(const char *start, const char *colon, const char *end)
    {                               //添加一个头部信息
        string field(start, colon); //header域
        ++colon;
        //去除左空格
        while (colon < end && isspace(*colon)) //header值
            ++colon;
        string value(colon, end);
        //去除右空格
        while (!value.empty() && isspace(value[value.size() - 1]))
            value.resize(value.size() - 1);
        headers_[field] = value; //将value的值保存到headers中
    }

    string getHeader(const string &field) const //根据头域返回值
    {
        string result;
        std::map<string, string>::const_iterator it = headers_.find(field);
        if (it != headers_.end())
            result = it->second;
        return result;
    }

    const std::map<string, string> &headers() const { return headers_; } //返回整个头域

    void swap(HttpRequest &that) //交换数据成员
    {
        std::swap(method_, that.method_);
        path_.swap(that.path_);
        query_.swap(that.query_);
        headers_.swap(that.headers_);
    }

private:
    Method method_; //请求方法
    string path_;   //请求路径
    string query_;
    std::map<string, string> headers_; //header列表
};

} // namespace net
} // namespace ssxrver

#endif
