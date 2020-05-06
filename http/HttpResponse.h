#ifndef SSXRVER_HTTP_HTTPRESPONSE_H
#define SSXRVER_HTTP_HTTPRESPONSE_H
#include <map>
namespace ssxrver
{
namespace net
{
class Buffer;
using std::string;
class HttpResponse //http相应封装的类
{
public:
    enum HttpStatus
    {
        kUnknown,                  //还有很多没实现
        k2000k = 200,              //成功
        k301MovePermanently = 301, //错误重定向，请求的页面永久转移到另一个地址
        k404BadRequest = 400,      //错误的请求，语法格式有问题，服务器无法处理此请求
        k404NotFound = 404         //请求的网页不存在
    };

    explicit HttpResponse(bool close)
        : statusCode_(kUnknown),
          closeConnection_(close)
    {
    }

    void setStatusCode(HttpStatus code) { statusCode_ = code; }
    void setStatusMessage(const string &message) { statusMessage_ = message; }
    void setCloseConnection(bool on) { closeConnection_ = on; }
    bool closeConnection() const { return closeConnection_; }
    //设置文档的媒体类型
    void setContentType(const string &contentType) { addHeader("Content-Type", contentType); }
    void addHeader(const string &key, const string &value) { headers_[key] = value; }
    void setBody(const string &body) { body_ = body; }
    void appendToBuffer(Buffer *output) const; //将httpresponse添加到buffer
private:
    std::map<string, string> headers_; //header列表
    HttpStatus statusCode_;            //状态响应码
    string statusMessage_;             //状态响应码对应的文本信息
    bool closeConnection_;             //是否关闭连接
    string body_;                      //实体
};

} // namespace net
} // namespace ssxrver
#endif
