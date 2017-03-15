#include "InetAddress.h"
#include <string.h> // bzero
#include <assert.h>
#include "SocketsOps.h"

namespace checkking {
namespace rpc {

uint32_t InetAddress::ipNetEndian() const
{
    assert(family() == AF_INET);
    return _addr.sin_addr.s_addr;
}

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

std::string InetAddress::toIpPort() const
{
    char buf[64] = "";
    sockets::toIpPort(buf, sizeof buf, getSockAddr());
    return buf;
}

std::string InetAddress::toIp() const
{
    char buf[64] = "";
    sockets::toIp(buf, sizeof buf, getSockAddr());
    return buf;
}

uint16_t InetAddress::toPort() const
{
    return sockets::networkToHost16(portNetEndian());
}
} // namespace rpc
} // namespace checkking
