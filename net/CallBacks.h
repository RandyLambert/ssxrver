//
// Created by randylambert on 2020/8/8.
//

#ifndef SSXRVER_NET_CALLBACKS_H
#define SSXRVER_NET_CALLBACKS_H

#include <functional>
#include <memory>

namespace ssxrver
{
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    namespace net
    {
        class Buffer;
        class TcpConnection;
        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
        using ConnectionCallback = std::function<void (const TcpConnectionPtr&)>;
        using CloseCallback = std::function<void (const TcpConnectionPtr&)>;
        using WriteCompleteCallback = std::function<void (const TcpConnectionPtr&)>;

        using MessageCallback = std::function<void (const TcpConnectionPtr&,
                                    Buffer*)>;

        void defaultConnectionCallback(const TcpConnectionPtr& conn);
        void defaultMessageCallback(const TcpConnectionPtr& conn,
                                    Buffer* buffer);

    }  // namespace net
}  // namespace ssxrver


#endif //SSXRVER_NET_CALLBACKS_H
