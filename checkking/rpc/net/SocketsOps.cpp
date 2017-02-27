#include "SocketsOps.h"

#include <fcntl.h>
#include <sys/socket.h>
#include<boost/implicit_cast.hpp> // implicit_cast
#include <stdio.h> // snprintf
#include <string.h> // bzero
#include <errno.h> // errno
#include <unistd.h>
#include "Logging.h"

namespace checkking {
namespace rpc {

typedef struct sockaddr SA;
const SA* sockaddr_cast(const struct sockaddr_in* addr) {
    return static_cast<const SA*>(boost::implicit_cast<const void*>(addr));
}
SA* sockaddr_cast(struct sockaddr_in* addr) {
    return static_cast<SA*>(boost::implicit_cast<void*>(addr));
}

void setNonBlockAndCloseOnExec(int sockfd) {
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags != O_NONBLOCK;
    int ret = ::fcntl(sockfd, F_SETFL, flags);
    flags = ::fcntl(sockfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(sockfd, F_SETFD, flags);
}

int sockets::createNonblockingOrDie() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG_FATAL << "create nonblcok socket falied!";
    }

    return sockfd;
}

void sockets::bindOrDie(int sockfd, const struct sockaddr_in& addr) {
    int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof addr);
    if (ret < 0) {
        LOG_FATAL << "bind socket failed!";
    }
}

void sockets::listenOrDie(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0) {
        LOG_FATAL << "listen socket failed!";
    }
}

int sockets::accept(int sockfd, struct sockaddr_in* addr) {
    socklen_t addrlen = sizeof *addr;
#if VALGRID
    int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
    setNonBlockAndCloseOnExec(connfd);
#else
    int connfd = ::accept4(sockfd, sockaddr_cast(addr),
            &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    if (connfd < 0) {
    
    }
    return connfd;
}
void sockets::close(int sockfd) {
    if (::close(sockfd) < 0) {
        LOG_FATAL << "close socket failed!";
    }
}

void sockets::toHostPort(char* buf, const size_t size, const struct sockaddr_in& addr) {
    char host[INET_ADDRSTRLEN] = "INVALID";
    ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof host);
    uint16_t port = sockets::networkToHost16(addr.sin_port);
    snprintf(buf, size, "%s:%u", host, port);
}
void sockets::fromHostPort(const char* ip, uint16_t port, struct sockaddr_in* addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = sockets::hostToNetwork16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
        LOG_FATAL << "fromHostPort failed!";
    }
}

struct sockaddr_in sockets::getLocalAddr(int sockfd) {
    struct sockaddr_in localaddr;
    bzero(&localaddr, sizeof localaddr);
    socklen_t addrlen = sizeof(localaddr);
    if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
        LOG_FATAL << "getLocalAddr failed!";
    }
    return localaddr;
}

int sockets::getSocketError(int sockfd) {
    int optval;
    socklen_t optlen = sizeof optval;
    if (::getsockopt(sockfd, SOL_SOCKET,  SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
}
} // namespace rpc
} // namespace checkking
