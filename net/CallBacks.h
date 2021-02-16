/*
 * @Date: 2020-08-09 21:05:35
 * @LastEditors: OBKoro1
 * @LastEditTime: 2021-02-05 22:42:22
 * @FilePath: /ssxrver/net/CallBacks.h
 * @Auther: SShouxun
 * @GitHub: https://github.com/RandyLambert
 */
//
// Created by randylambert on 2020/8/8.
//

#ifndef SSXRVER_NET_CALLBACKS_H
#define SSXRVER_NET_CALLBACKS_H

#include <functional>
#include <memory>

namespace ssxrver::net
{
    class Buffer;
    class TcpConnection;
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void (const TcpConnectionPtr&)>;
    using CloseCallback = std::function<void (const TcpConnectionPtr&)>;
    using WriteCompleteCallback = std::function<void (const TcpConnectionPtr&)>;
    using TimerCallback = std::function<void()>;
    using MessageCallback = std::function<void (const TcpConnectionPtr&,
                                Buffer*)>;

    void defaultConnectionCallback(const TcpConnectionPtr& conn);
    void defaultMessageCallback(const TcpConnectionPtr& conn,
                                Buffer* buffer);
}  // namespace ssxrver::net


#endif //SSXRVER_NET_CALLBACKS_H
