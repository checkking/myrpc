#include "Socket.h"
#include "SocketsOps.h"
#include <string.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

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

void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}

void Socket::shutdownWrite() {
    sockets::shutdownWrite(_fd);
}
} // namespace rpc
} // namespace checkking
