#ifndef SSXRVER_HTTP_HTTPPARSER_H
#define SSXRVER_HTTP_HTTPPARSER_H
#include <memory>
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "http11_parser.h"
#include "httpclient_parser.h"
namespace ssxrver
{
namespace net
{

class Buffer;
class HttpRequestParser
{
public:
    HttpRequestParser();

    int isFinished();                              //是否解析完成
    int hasError();                                //是否有错
    HttpRequest &getRequest() { return request_; } //获取httprequest
    const http_parser &getParser() const { return parser_; }
    void setError(int v) { error_ = v; }
    int getError() const { return error_; }
    size_t execute(Buffer *data);
    uint64_t getContentLength();
    void reset();

private:
    http_parser parser_;
    HttpRequest request_;
    int error_;
};

class HttpResponseParser
{
public:
    HttpResponseParser();

    int isFinished();
    int hasError();
    void setError(int v) { error_ = v; }
    int getError() const { return error_; }
    size_t execute(Buffer *data);
    HttpResponse &getResponse() { return response_; } //获取httprequest
    const httpclient_parser &getParser() const { return parser_; }
    void reset();

private:
    httpclient_parser parser_;
    HttpResponse response_;
    int error_;
};

} // namespace net
} // namespace ssxrver

#endif
