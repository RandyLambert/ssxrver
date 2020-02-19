#include "SocketOps.h"
using namespace ssxrver::net;
void socketops::bindAndlisten(int sockfd,const struct sockaddr_in &addr)
{
    if(bind(sockfd,(struct sockaddr*)&addr,static_cast<socklen_t>(sizeof addr)) < 0){
        LOG_SYSFATAL <<"socketops error";
    }

}
