#ifndef SSXRVER_NET_SOCKETOPS_H
#define SSXRVER_NET_SOCKETOPS_H
#include <arpa/inet.h>
#include "Channel.h"
namespace ssxrver
{
namespace net
{
namespace socketops
{

int createNonblockingOrDie(); //创建一个非阻塞的套接字

int connect(int sockfd, const struct sockaddr_in &addr);    //链接
void bindOrDie(int sockfd, const struct sockaddr_in &addr); //绑定监听
void listenOrDie(int sockfd);
int accept(int sockfd, struct sockaddr_in *addr);
ssize_t read(int sockfd, void *buf, size_t count);
ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
ssize_t write(int sockfd, const void *buf, size_t count);
void close(int sockfd);
void shutdownWrite(int sockfd);
void setKeepAlive(int sockfd, bool on);
void setTcpNoDelay(int sockfd_, bool on); //禁用或开启negle
void setReuseAddr(int sockfd_, bool on);  //地址复用,禁用或开启
void setReusePort(int sockfd_, bool on);
int getSocketError(int sockfd);

struct sockaddr_in getLocalAddr(int sockfd);
struct sockaddr_in getPeerAddr(int sockfd);

} // namespace socketops
} // namespace net
} // namespace ssxrver
#endif
