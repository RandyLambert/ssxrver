// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/sserver/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef sserver_NET_INETADDRESS_H
#define sserver_NET_INETADDRESS_H

#include "../sserver_base/StringPiece.h"

#include <netinet/in.h>

namespace sserver
{
namespace net
{

///对于网际地址的封装
/// Wrapper of sockaddr_in.
///
/// This is an POD interface class.
class InetAddress
{
public:
  /// Constructs an endpoint with given port number.
  /// Mostly used in TcpServer listening.
  /// 仅仅只是指定了port，不指定ip，则ip为INADDR_ANY
  explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);

  /// Constructs an endpoint with given ip and port.
  /// @c ip should be "1.2.3.4"
  InetAddress(StringArg ip, uint16_t port);//构造一个地址，ip和端口都指定

  /// Constructs an endpoint with given struct @c sockaddr_in
  /// Mostly used when accepting new connections
  InetAddress(const struct sockaddr_in &addr)
      : addr_(addr)//对socketaddrin的封装
  {
  }

  std::string toIp() const;//转为ip
  std::string toIpPort() const;//转为ip端口
  uint16_t toPort() const;

  // default copy/assignment are Okay

  const struct sockaddr_in &getSockAddrInet() const { return addr_; }
  void setSockAddrInet(const struct sockaddr_in &addr) { addr_ = addr; }

  uint32_t ipNetEndian() const { return addr_.sin_addr.s_addr; }//返回网络字节序的32位ip
  uint16_t portNetEndian() const { return addr_.sin_port; }//返回网络字节序的端口

  // resolve hostname to IP address, not changing port or sin_family
  // return true on success.
  // thread safe
  static bool resolve(StringArg hostname, InetAddress *result);
  // static std::vector<InetAddress> resolveAll(const char* hostname, uint16_t port = 0);

private:
  struct sockaddr_in addr_;
};

} // namespace net
} // namespace sserver

#endif // SSERVER_INETADDRESS_H
