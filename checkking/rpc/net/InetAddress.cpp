#include "InetAddress.h"
#include <string.h> // bzero
#include "SocketsOps.h"

namespace checkking {
namespace rpc {
InetAddress::InetAddress(uint16_t port) {
    bzero(&_addr, sizeof _addr);
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = sockets::hostToNetwork32(INADDR_ANY);
    _addr.sin_port = sockets::hostToNetwork16(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port) {
    bzero(&_addr, sizeof _addr);
    sockets::fromHostPort(ip.c_str(), port, &_addr);
}

std::string InetAddress::toHostPort() const {
    char buf[32];
    sockets::toHostPort(buf, sizeof buf, _addr);

    return buf;
}
} // namespace rpc
} // namespace checkking
