// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/sserver/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef SSERVER_ACCEPTOR_H
#define SSERVER_ACCEPTOR_H

#include <functional>
#include "Channel.h"
#include "Socket.h"

namespace sserver
{
namespace net
{

class EventLoop;
class InetAddress;

///
/// Acceptor of incoming TCP connections.
/// 用于接受tcp套接字的链接
class Acceptor
{
public:
  typedef std::function<void(int sockfd,
                             const InetAddress &)>
      NewConnectionCallback;

  Acceptor(const Acceptor &) = delete;
  Acceptor &operator=(const Acceptor &) = delete;

  Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
  ~Acceptor();

  void setNewConnectionCallback(const NewConnectionCallback &cb) //设置新连接来了需要处理的回调函数，比如：打印新连接啥啥啥来了
  {
    newConnectionCallback_ = cb;
  }

  bool listenning() const { return listenning_; }
  void listen();

private:
  void handleRead();

  EventLoop *loop_;       //accept所属的eventloop
  Socket acceptSocket_;   //是listening socket（即server socket）
  Channel acceptChannel_; //channel用于观察此socket的readable事件，并会带哦accept::handleread(),后者调用accept(2)来接受新连接，并回调用户callback
  //通道回去观察accept的可读事件
  NewConnectionCallback newConnectionCallback_;
  bool listenning_; //所属的eventloop是否处于监听状态
  int idleFd_;
};

} // namespace net
} // namespace sserver

#endif // SSERVER_ACCEPTOR_H
