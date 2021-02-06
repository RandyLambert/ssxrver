#ifndef SSXRVER_HTTP_HTTPREQUEST_H
#define SSXRVER_HTTP_HTTPREQUEST_H
#include <map>
#include <assert.h>
#include <cstdio>
#include <string>
#include <boost/lexical_cast.hpp>
#include "../base/Logging.h"

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
        kPost,
    };
    explicit HttpRequest()
        : method_(kInvalid),
          version_(0x00)
    {
    }

    bool setMethod(const char *start, const size_t length)
    {
        assert(method_ == kInvalid);
        if (strncmp(start, "GET", length) == 0)
        {
            method_ = kGet;
        }
        else if (strncmp(start, "HEAD", length) == 0)
        {
            method_ = kHead;
        }
        else if (strncmp(start, "POST", length) == 0)
        {
            method_ = kPost;
        }
        else
        {
            method_ = kInvalid;
        }
        return method_ != kInvalid; //看是否请求成功
    }

    bool setVersion(const char *start, const size_t length)
    {
        assert(version_ == 0x00);
        if (strncmp(start, "HTTP/1.1", length) == 0)
        {
            version_ = 0x11;
        }
        else if (strncmp(start, "HTTP/1.0", length) == 0)
        {
            version_ = 0x10;
        }
        else
        {
            version_ = 0x00;
        }
        return version_ != 0x00;
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
        case kPost:
            result = "POST";
            break;
        default:
            break;
        }
        return result;
    }

    void setPath(const char *start, const size_t length) { path_.assign(start, length); } //设置路径
    const std::string &path() const { return path_; }
    void setQuery(const char *start, const size_t length) { query_.assign(start, length); }
    const string &query() const { return query_; }
    void addBody(const char *start, const char *end) { body_.append(start, end); }
    const string &body() const { return body_; }
    void setHeader(std::string_view key, std::string_view value) { headers_.insert({key.data(),value.data()}); }

    template <class T>
    T getHeader(const string &key, const T &def = T()) const //根据头域返回值
    {
        auto value = headers_.find(key);
        if (value == headers_.end())
        {
            return def;
        }
        try
        {
            return boost::lexical_cast<T>(value->second);
        }
        catch (...)
        {
            return def;
        }
    }

    const std::map<string, string> &headers() const { return headers_; } //返回整个头域

    void swap(HttpRequest &that) //交换数据成员
    {
        std::swap(method_, that.method_);
        std::swap(version_, that.version_);
        path_.swap(that.path_);
        query_.swap(that.query_);
        body_.swap(that.body_);
        headers_.swap(that.headers_);
    }

private:
    Method method_;                    //请求方法
    uint8_t version_;                  //http版本
    string path_;                      //请求路径
    string query_;                     //查询参数
    string body_;                      //请求实体
    std::map<string, string> headers_; //header列表
};

} // namespace net
} // namespace ssxrver

#endif
