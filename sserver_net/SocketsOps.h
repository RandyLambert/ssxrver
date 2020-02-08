// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/sserver/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef SSERVER_SOCKETSOPS_H
#define SSERVER_SOCKETSOPS_H

#include <arpa/inet.h>

namespace sserver
{
namespace net
{
namespace sockets
{ //封装了socket相关的系统调用（全局函数）

///
/// Creates a non-blocking socket file descriptor,
/// abort if any error.
int createNonblockingOrDie(); //创建一个非阻塞的套接字

int connect(int sockfd, const struct sockaddr_in &addr);    //链接
void bindOrDie(int sockfd, const struct sockaddr_in &addr); //绑定
void listenOrDie(int sockfd);                               //监听
int accept(int sockfd, struct sockaddr_in *addr);
ssize_t read(int sockfd, void *buf, size_t count);
ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
ssize_t write(int sockfd, const void *buf, size_t count);
void close(int sockfd);
void shutdownWrite(int sockfd);

void toIpPort(char *buf, size_t size,
              const struct sockaddr_in &addr);
void toIp(char *buf, size_t size,
          const struct sockaddr_in &addr);
void fromIpPort(const char *ip, uint16_t port,
                struct sockaddr_in *addr);

int getSocketError(int sockfd);

const struct sockaddr *sockaddr_cast(const struct sockaddr_in *addr);
struct sockaddr *sockaddr_cast(struct sockaddr_in *addr);
const struct sockaddr_in *sockaddr_in_cast(const struct sockaddr *addr);
struct sockaddr_in *sockaddr_in_cast(struct sockaddr *addr);

struct sockaddr_in getLocalAddr(int sockfd);
struct sockaddr_in getPeerAddr(int sockfd);
bool isSelfConnect(int sockfd);

} // namespace sockets
} // namespace net
} // namespace sserver

#endif // SSERVER_SOCKETSOPS_H
