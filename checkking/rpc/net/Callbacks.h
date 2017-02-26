#ifndef CHECKKING_RPC_NET_CALLBACKS_H
#define CHECKKING_RPC_NET_CALLBACKS_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

namespace checkking {
namespace rpc {
class TcpConnection;
typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef boost::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef boost::function<void (const TcpConnectionPtr&, const char* data, size_t len)> MessageCallback;
typedef boost::function<void (const TcpConnectionPtr&)> CloseCallback;
} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_ACCEPTOR_H
