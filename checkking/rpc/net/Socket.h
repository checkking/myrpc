#ifndef CHECKKING_RPC_NET_SOCKET_H
#define CHECKKING_RPC_NET_SOCKET_H

#include <boost/noncopyable.hpp>
#include "InetAddress.h"

namespace checkking {
namespace rpc {
class Socket: boost::noncopyable {
public:
    explicit Socket(int fd):_fd(fd) {}
    ~Socket() {}
    
    int fd() const {
        return _fd;
    }

    void bindAddress(const InetAddress& localaddr);

    void listen();

    int accept(InetAddress* peeraddr);

    void setReuseAddr(bool on);

private:
int _fd;
}; // class Socket

} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_SOCKET_H
