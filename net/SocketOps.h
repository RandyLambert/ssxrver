/*
 * @Date: 2020-08-06 17:33:41
 * @LastEditors: OBKoro1
 * @LastEditTime: 2021-02-04 21:39:35
 * @FilePath: /ssxrver/net/SocketOps.h
 * @Auther: SShouxun
 * @GitHub: https://github.com/RandyLambert
 */
#ifndef SSXRVER_NET_SOCKETOPS_H
#define SSXRVER_NET_SOCKETOPS_H
#include <arpa/inet.h>
namespace ssxrver::net::socketops
{

int createNonblockingOrDie(); //创建一个非阻塞的套接字

int connect(int sockfd, struct sockaddr_in* addr);    //链接
void bindOrDie(int sockfd, struct sockaddr_in* addr); //绑定监听
void listenOrDie(int sockfd);
int accept(int sockfd, struct sockaddr_in *addr);
ssize_t read(int sockfd, void *buf, size_t count);
ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
ssize_t write(int sockfd, const void *buf, size_t count);
ssize_t sendfile(int outFd, int inFd, off_t *offset, size_t count);
void close(int sockfd);
void shutdownWrite(int sockfd);
void setKeepAlive(int sockFd, bool on);
void setTcpNoDelay(int sockFd, bool on); //禁用或开启negle
void setReuseAddr(int sockFd, bool on);  //地址复用,禁用或开启
void setReusePort(int sockFd, bool on);
int getSocketError(int sockfd);
struct sockaddr_in getLocalAddr(int sockFd);
struct sockaddr_in getPeerAddr(int sockFd);

} // namespace ssxrver::net::socketops
#endif
