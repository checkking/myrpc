#ifndef CHECKKING_RPC_NET_INETADDRESS_H
#define CHECKKING_RPC_NET_INETADDRESS_H
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include "SocketsOps.h"

namespace checkking {
namespace rpc {
class InetAddress {
public:
    explicit InetAddress(uint16_t port);

    InetAddress(const std::string& ip, uint16_t port);

    explicit InetAddress(const struct sockaddr_in& addr): _addr(addr) {}
    explicit InetAddress(const struct sockaddr_in6& addr): _addr6(addr) {}

    std::string toHostPort() const;

    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;

    const struct sockaddr_in& getSockaddrIn() const {
        return _addr;
    }
    const struct sockaddr* getSockAddr() const { 
        return sockets::sockaddr_cast(&_addr6);
    }
    void setSockAddrIn(const struct sockaddr_in& addr) {
        _addr = addr;
    }
    sa_family_t family() const {
        return _addr.sin_family;
    }

    uint32_t ipNetEndian() const;
    uint16_t portNetEndian() const { return _addr.sin_port; }

private:
    union
    {
        struct sockaddr_in _addr;
        struct sockaddr_in6 _addr6;
    };
}; // class InentAddress
} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_INETADDRESS_H
