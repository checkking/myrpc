#ifndef CHECKKING_RPC_NET_CALLBACKS_H
#define CHECKKING_RPC_NET_CALLBACKS_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include "Buffer.h"
#include "Timestamp.h"

namespace checkking {
namespace rpc {
class TcpConnection;
typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef boost::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef boost::function<void (const TcpConnectionPtr&, Buffer*, Timestamp)> MessageCallback;
typedef boost::function<void (const TcpConnectionPtr&)> CloseCallback;
typedef boost::function<void()> TimerCallback;

template<typename To, typename From>
inline ::boost::shared_ptr<To> down_pointer_cast(const ::boost::shared_ptr<From>& f) {
    if (false) {
        implicit_cast<From*, To*>(0);
    }

#ifndef NDEBUG
    assert(f == NULL || dynamic_cast<To*>(get_pointer(f)) != NULL);
#endif
    return ::boost::static_pointer_cast<To>(f);
}
} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_ACCEPTOR_H
