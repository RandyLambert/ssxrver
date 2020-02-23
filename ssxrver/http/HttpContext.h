#ifndef SSXRVER_HTTP_HTTPCONTEXT_H
#define SSXRVER_HTTP_HTTPCONTEXT_H

#include "HttpRequest.h"

namespace ssxrver
{
namespace net
{

class HttpContext
{//http协议解析类的封装
public:
    enum HttpRequestParseState
    {
        kExpectRequestLine, //正处于解析请求行的状态
        kExpectHeaders,     //正处于解析头部状态
        kExpectBody,        //处于解析实体状态，未实现
        kGotAll             //全部解析完毕
    };
    HttpContext() 
        : state_(kExpectRequestLine)
    {
    }
    //将状态设置为上述几个状态
    bool expectRequestLine() const { return state_ == kExpectRequestLine; }//将状态设置成上述几个状态
    bool expectHeaders() const {return state_ == kExpectHeaders;}
    bool expectBody() const { return state_ == kExpectBody; }
    bool gotAll() const {return state_ == kGotAll; }
    void receiveRequestLine(){ state_ = kExpectHeaders; }
    void receiveHeaders() { state_ = kGotAll; }

    void reset()//重置httpconrext状态
    {
        state_ = kExpectRequestLine; //重置为初始状态
        HttpRequest dummy;
        request_.swap(dummy); //将当前对象置空
    }
    const HttpRequest &request() const { return request_; }
    HttpRequest &request(){return request_;}//返回请求

private:
    HttpRequestParseState state_; //请求解析状态
    HttpRequest request_;         //http请求
};
}
}
#endif
