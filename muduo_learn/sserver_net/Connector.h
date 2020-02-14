// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/sserver/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef SSERVER_CONNECTOR_H
#define SSERVER_CONNECTOR_H

#include "InetAddress.h"

#include <functional>
#include <memory>

namespace sserver
{
namespace net
{

class Channel;
class EventLoop;
//主动发起链接，带有自动重连功能
class Connector : public std::enable_shared_from_this<Connector>
{
public:
  typedef std::function<void(int sockfd)> NewConnectionCallback;

  Connector(const Connector &) = delete;
  Connector &operator=(const Connector &) = delete;

  Connector(EventLoop *loop, const InetAddress &serverAddr);
  ~Connector();

  void setNewConnectionCallback(const NewConnectionCallback &cb)
  {
    newConnectionCallback_ = cb;
  }

  void start();   // can be called in any thread
  void restart(); // must be called in loop thread
  void stop();    // can be called in any thread

  const InetAddress &serverAddress() const { return serverAddr_; }

private:
  enum States
  {
    kDisconnected,
    kConnecting,
    kConnected
  };
  static const int kMaxRetryDelayMs = 30 * 1000; //30秒，最大重连延迟时间
  static const int kInitRetryDelayMs = 500;      //0.5秒，初始状态，连接不上，0.5秒后重连

  void setState(States s) { state_ = s; } //设置状态
  void startInLoop();
  void stopInLoop();
  void connect();
  void connecting(int sockfd);
  void handleWrite();
  void handleError();
  void retry(int sockfd);
  int removeAndResetChannel();
  void resetChannel();

  EventLoop *loop_;                             //所属eventloop
  InetAddress serverAddr_;                      //服务端地址
  bool connect_;                                // atomic
  States state_;                                // FIXME: use atomic variable
  std::unique_ptr<Channel> channel_;            //connector所对应的channel
  NewConnectionCallback newConnectionCallback_; //连接成功回调函数
  int retryDelayMs_;                            //重连延迟时间（单位：毫秒）
};

} // namespace net
} // namespace sserver

#endif // SSERVER_CONNECTOR_H
