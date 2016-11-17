#include "Socket.h"
#include "SocketsOps.h"
#include <string.h>

namespace checkking {
namespace rpc {
void Socket::bindAddress(const InetAddress& localaddr) {
    sockets::bindOrDie(_fd, localaddr.getSockaddrIn());
}

void Socket::listen() {
    sockets::listenOrDie(_fd);
}

int Socket::accept(InetAddress* peeraddr) {
    struct sockaddr_in addr;
    bzero(&addr, sizeof addr);
    int connfd = sockets::accept(_fd, &addr);
    if (connfd >= 0) {
        peeraddr->setSockAddrIn(addr);
    }
    return connfd;
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}
} // namespace rpc
} // namespace checkking
