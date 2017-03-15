#include "SocketsOps.h"

#include <fcntl.h>
#include <sys/socket.h>
#include<boost/implicit_cast.hpp> // implicit_cast
#include <stdio.h> // snprintf
#include <string.h> // bzero
#include <errno.h> // errno
#include <assert.h>
#include <unistd.h>
#include "Logging.h"

namespace checkking {
namespace rpc {

typedef struct sockaddr SA;
const SA* sockets::sockaddr_cast(const struct sockaddr_in* addr) {
    return static_cast<const SA*>(boost::implicit_cast<const void*>(addr));
}
SA* sockets::sockaddr_cast(struct sockaddr_in* addr) {
    return static_cast<SA*>(boost::implicit_cast<void*>(addr));
}
const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in6* addr) {
    return static_cast<const struct sockaddr*>(boost::implicit_cast<const void*>(addr));
}
struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in6* addr) {
    return static_cast<struct sockaddr*>(boost::implicit_cast<void*>(addr));
}

const struct sockaddr_in* sockets::sockaddr_in_cast(const struct sockaddr* addr) {
    return static_cast<const struct sockaddr_in*>(boost::implicit_cast<const void*>(addr));
}

const struct sockaddr_in6* sockets::sockaddr_in6_cast(const struct sockaddr* addr) {
    return static_cast<const struct sockaddr_in6*>(boost::implicit_cast<const void*>(addr));
}

void setNonBlockAndCloseOnExec(int sockfd) {
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags != O_NONBLOCK;
    int ret = ::fcntl(sockfd, F_SETFL, flags);
    flags = ::fcntl(sockfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(sockfd, F_SETFD, flags);
}

int sockets::createNonblockingOrDie(sa_family_t family) {
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG_FATAL << "create nonblcok socket falied!";
    }

    return sockfd;
}

void sockets::bindOrDie(int sockfd, const struct sockaddr* addr) {
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
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

int sockets::accept(int sockfd, struct sockaddr_in6* addr) {
    socklen_t addrlen = sizeof *addr;
#if VALGRID
    int connfd = ::accept(sockfd, sockets::sockaddr_cast(addr), &addrlen);
    setNonBlockAndCloseOnExec(connfd);
#else
    int connfd = ::accept4(sockfd, sockets::sockaddr_cast(addr),
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

void sockets::shutdownWrite(int sockfd) {
    if (::shutdown(sockfd, SHUT_WR) < 0) {
        LOG_FATAL << "shutdownWrite failed!";
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

struct sockaddr_in6 sockets::getLocalAddr(int sockfd) {
    struct sockaddr_in6 localaddr;
    bzero(&localaddr, sizeof localaddr);
    socklen_t addrlen = sizeof(localaddr);
    if (::getsockname(sockfd, sockets::sockaddr_cast(&localaddr), &addrlen) < 0) {
        LOG_ERROR << "getLocalAddr failed!";
    }
    return localaddr;
}

struct sockaddr_in6 sockets::getPeerAddr(int sockfd) {
    struct sockaddr_in6 peeraddr;
    bzero(&peeraddr, sizeof peeraddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
    if (::getpeername(sockfd, sockets::sockaddr_cast(&peeraddr), &addrlen) < 0) {
        LOG_ERROR << "sockets::getPeerAddr";
    }
    return peeraddr;
}

void sockets::toIpPort(char* buf, size_t size,
                       const struct sockaddr* addr)
{
  toIp(buf,size, addr);
  size_t end = ::strlen(buf);
  const struct sockaddr_in* addr4 = sockets::sockaddr_in_cast(addr);
  uint16_t port = sockets::networkToHost16(addr4->sin_port);
  assert(size > end);
  snprintf(buf+end, size-end, ":%u", port);
}

void sockets::toIp(char* buf, size_t size,
                   const struct sockaddr* addr)
{
  if (addr->sa_family == AF_INET)
  {
    assert(size >= INET_ADDRSTRLEN);
    const struct sockaddr_in* addr4 = sockets::sockaddr_in_cast(addr);
    ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
  }
  else if (addr->sa_family == AF_INET6)
  {
    assert(size >= INET6_ADDRSTRLEN);
    const struct sockaddr_in6* addr6 = sockets::sockaddr_in6_cast(addr);
    ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
  }
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

int sockets::connect(int sockfd, const struct sockaddr* addr) {
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

bool sockets::isSelfConnect(int sockfd) {
    struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
    struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
    if (localaddr.sin6_family == AF_INET) {
        const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
        const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
        return laddr4->sin_port == raddr4->sin_port
            && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
    } else if (localaddr.sin6_family == AF_INET6) {
        return localaddr.sin6_port == peeraddr.sin6_port
            && memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof localaddr.sin6_addr) == 0;
    } else {
        return false;
    }
}
} // namespace rpc
} // namespace checkking
