// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/sserver/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef SSERVER_HTTP_HTTPCONTEXT_H
#define SSERVER_HTTP_HTTPCONTEXT_H

#include "HttpRequest.h"

namespace sserver
{
namespace net
{

class HttpContext
{ //http协议解析类的封装
public:
    enum HttpRequestParseState
    {
        kExpectRequestLine, //正处于解析请求行的状态
        kExpectHeaders,     //正处于解析头部状态
        kExpectBody,        //正处于解析实体状态
        kGotAll,            //全部解析完毕
    };

    HttpContext()
        : state_(kExpectRequestLine)
    {
    }

    // default copy-ctor, dtor and assignment are fine

    bool expectRequestLine() const //将状态设置为上述几个状态
    {
        return state_ == kExpectRequestLine;
    }

    bool expectHeaders() const
    {
        return state_ == kExpectHeaders;
    }

    bool expectBody() const
    {
        return state_ == kExpectBody;
    }

    bool gotAll() const
    {
        return state_ == kGotAll;
    }

    void receiveRequestLine()
    {
        state_ = kExpectHeaders;
    }

    void receiveHeaders()
    {
        state_ = kGotAll;
    } // FIXME

    //重置httpcontext状态
    void reset()
    {
        state_ = kExpectRequestLine; //重置为初始状态
        HttpRequest dummy;
        request_.swap(dummy); //将当前对象置空
    }

    const HttpRequest &request() const
    {
        return request_;
    }

    HttpRequest &request() //返回请求
    {
        return request_;
    }

private:
    HttpRequestParseState state_; //请求解析状态
    HttpRequest request_;         //http请求
};

} // namespace net
} // namespace sserver

#endif // SSERVER_HTTP_HTTPCONTEXT_H
