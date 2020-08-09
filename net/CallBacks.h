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
        typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
        typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
        typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
        typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;

        typedef std::function<void (const TcpConnectionPtr&,
                                    Buffer*)> MessageCallback;

        void defaultConnectionCallback(const TcpConnectionPtr& conn);
        void defaultMessageCallback(const TcpConnectionPtr& conn,
                                    Buffer* buffer);

    }  // namespace net
}  // namespace ssxrver


#endif //SSXRVER_NET_CALLBACKS_H
