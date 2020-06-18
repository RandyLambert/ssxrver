#include <map>
#include "../base/Logging.h"
#include "../net/Buffer.h"
#include "HttpParser.h"
using namespace ssxrver;
using namespace ssxrver::net;
namespace
{
void onRequestMethod(void *data, const char *at, size_t length)
{
    HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
    bool flag = parser->getRequest().setMethod(at, length);
    if (flag == false)
    {
        LOG_ERROR << "invalid http request method: " << string(at, length);
        parser->setError(1000);
    }
}

void onRequestUri(void *data, const char *at, size_t length)
{
    (void)data;
    (void)at;
    (void)length;
}

void onRequestFragment(void *data, const char *at, size_t length)
{
    (void)data;
    (void)at;
    (void)length;
    /* HttpRequestParser *parser = static_cast<HttpRequestParser *>(data); */
    /* parser->getRequest().setFragment(at, length); */
}

void onRequestPath(void *data, const char *at, size_t length)
{
    HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
    parser->getRequest().setPath(at, length);
}
void onRequestQuery(void *data, const char *at, size_t length)
{
    HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
    parser->getRequest().setQuery(at, length);
}
void onRequestVersion(void *data, const char *at, size_t length)
{
    HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
    bool flag = parser->getRequest().setVersion(at, length);

    if (flag == false)
    {
        LOG_ERROR << "invalid http request version: " << string(at, length);
        parser->setError(1001);
    }
}
void onRequestHeaderDone(void *data, const char *at, size_t length)
{
    (void)data;
    (void)at;
    (void)length;
}

void onRequestHttpField(void *data, const char *field, size_t flen, const char *value, size_t vlen)
{
    HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
    if (flen == 0)
    {
        LOG_ERROR << "invalid http request field length == 0";
        return;
    }
    parser->getRequest().setHeader(std::string(field, flen), std::string(value, vlen));
}

/********************************************************************/

void onResponseReason(void *data, const char *at, size_t length)
{
    HttpResponseParser *parser = static_cast<HttpResponseParser *>(data);
    parser->getResponse().setStatusMessage(std::string(at, length));
}

void onResponseStatus(void *data, const char *at, size_t length)
{
    (void)length;
    HttpResponseParser *parser = static_cast<HttpResponseParser *>(data);
    ssxrver::net::HttpResponse::HttpStatus status = static_cast<ssxrver::net::HttpResponse::HttpStatus>(atoi(at));
    parser->getResponse().setStatusCode(status);
}

void onResponseChunk(void *data, const char *at, size_t length)
{
    (void)data;
    (void)at;
    (void)length;
}

void onResponseVersion(void *data, const char *at, size_t length)
{
    HttpResponseParser *parser = static_cast<HttpResponseParser *>(data);
    bool flag = parser->getResponse().setVersion(at, length);
    if (flag == false)
    {
        LOG_ERROR << "invalid http response version: " << string(at, length);
        parser->setError(1001);
    }
}

void onResponseHeaderDone(void *data, const char *at, size_t length)
{
    (void)data;
    (void)at;
    (void)length;
}

void onResponseLastChunk(void *data, const char *at, size_t length)
{
    (void)data;
    (void)at;
    (void)length;
}

void onResponseHttpField(void *data, const char *field, size_t flen, const char *value, size_t vlen)
{
    HttpResponseParser *parser = static_cast<HttpResponseParser *>(data);
    if (flen == 0)
    {
        LOG_ERROR << "invalid http response field length == 0";
        return;
    }
    parser->getResponse().addHeader(std::string(field, flen), std::string(value, vlen));
}

} // namespace

HttpRequestParser::HttpRequestParser()
    : error_(0)
{
    http_parser_init(&parser_);
    parser_.request_method = onRequestMethod;
    parser_.request_uri = onRequestUri;
    parser_.fragment = onRequestFragment;
    parser_.request_path = onRequestPath;
    parser_.query_string = onRequestQuery;
    parser_.http_version = onRequestVersion;
    parser_.header_done = onRequestHeaderDone;
    parser_.http_field = onRequestHttpField;
    parser_.data = this;
}

uint64_t HttpRequestParser::getContentLength()
{
    return request_.getHeader<uint64_t>("Content-Length", 0);
}

size_t HttpRequestParser::execute(Buffer *data)
{
    if (!isFinished())
    {
        size_t offset = http_parser_execute(&parser_, data->peek(), data->readableBytes(), 0);
        data->retrieveUntil(data->peek() + offset);
        return offset;
    }
    return 0;
}

int HttpRequestParser::isFinished()
{
    return http_parser_finish(&parser_);
}

int HttpRequestParser::hasError()
{
    /* LOG_ERROR << error_ << "   " << http_parser_has_error(&parser_); */
    return error_ || http_parser_has_error(&parser_);
}
void HttpRequestParser::reset()
{
    http_parser_init(&parser_);
    HttpRequest dummy;
    request_.swap(dummy);
}

/*********************************************************onresponse181*/
HttpResponseParser::HttpResponseParser()
    : error_(0)
{
    httpclient_parser_init(&parser_);
    parser_.reason_phrase = onResponseReason;
    parser_.status_code = onResponseStatus;
    parser_.chunk_size = onResponseChunk;
    parser_.http_version = onResponseVersion;
    parser_.header_done = onResponseHeaderDone;
    parser_.last_chunk = onResponseLastChunk;
    parser_.http_field = onResponseHttpField;
    parser_.data = this;
}

size_t HttpResponseParser::execute(Buffer *data)
{
    if (!isFinished())
    {
        size_t offset = httpclient_parser_execute(&parser_, data->peek(), data->readableBytes(), 0);
        data->retrieveUntil(data->peek() + offset);
        return offset;
    }
    return 0;
}

int HttpResponseParser::isFinished()
{
    return httpclient_parser_finish(&parser_);
}

int HttpResponseParser::hasError()
{
    /* LOG_ERROR << error_ << "   " << httpclient_parser_has_error(&parser_); */
    return error_ || httpclient_parser_has_error(&parser_);
}

void HttpResponseParser::reset()
{
    httpclient_parser_init(&parser_);
    HttpResponse dummy;
    response_.swap(dummy);
}
