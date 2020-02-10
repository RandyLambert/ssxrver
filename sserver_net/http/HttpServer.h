// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/sserver/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef SSERVER_HTTP_HTTPSERVER_H
#define SSERVER_HTTP_HTTPSERVER_H

#include "../TcpServer.h"

namespace sserver
{
namespace net
{

class HttpRequest;
class HttpResponse;

/// A simple embeddable HTTP server designed for report status of a program.
/// It is not a fully HTTP 1.1 compliant server, but provides minimum features
/// that can communicate with HttpClient and Web browser.
/// It is synchronous, just like Java Servlet.
class HttpServer 
{
public:
    typedef std::function<void (const HttpRequest&,
                                HttpResponse*)> HttpCallback;

    HttpServer(const HttpServer&)=delete ;
    HttpServer& operator=(const HttpServer&)=delete ;

    HttpServer(EventLoop* loop,
               const InetAddress& listenAddr,
               const string& name,
               TcpServer::Option option = TcpServer::kNoReusePort);

    ~HttpServer();  // force out-line dtor, for scoped_ptr members.

    EventLoop* getLoop() const { return server_.getLoop(); }

    /// Not thread safe, callback be registered before calling start().
    void setHttpCallback(const HttpCallback& cb)
    {
        httpCallback_ = cb;
    }

    void setThreadNum(int numThreads)
    {
        server_.setThreadNum(numThreads);
    }

    void start();

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn,
                   Buffer* buf,
                   Timestamp receiveTime);
    void onRequest(const TcpConnectionPtr&, const HttpRequest&);

    TcpServer server_;
    HttpCallback httpCallback_;
};

}
}

#endif  // sserver_NET_HTTP_HTTPSERVER_H
