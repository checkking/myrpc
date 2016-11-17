#ifndef CHECKKING_RPC_NET_INETADDRESS_H
#define CHECKKING_RPC_NET_INETADDRESS_H
#include <boost/noncopyable.hpp>
#include <arpa/inet.h>
#include <string>

namespace checkking {
namespace rpc {
class InetAddress {
public:
    explicit InetAddress(uint16_t port);

    InetAddress(const std::string& ip, uint16_t port);

    InetAddress(const struct sockaddr_in& addr): _addr(addr) {}

    std::string toHostPort() const;

    const struct sockaddr_in& getSockaddrIn() const {
        return _addr;
    }
    void setSockAddrIn(const struct sockaddr_in& addr) {
        _addr = addr;
    }

private:
    struct sockaddr_in _addr;
}; // class InentAddress
} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_INETADDRESS_H
