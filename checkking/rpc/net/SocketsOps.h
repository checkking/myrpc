#ifndef CHECKKING_RPC_NET_SOCKETSOPS_H
#define CHECKKING_RPC_NET_SOCKETSOPS_H

#include <arpa/inet.h>
#include <endian.h>

namespace checkking {
namespace rpc {
namespace sockets {
inline uint64_t hostToNetwork64(uint64_t host64) {
    return htobe64(host64);
}

inline uint32_t hostToNetwork32(uint32_t host32) {
    return htonl(host32);
}

inline uint16_t hostToNetwork16(uint16_t host16) {
    return htons(host16);
}

inline uint64_t networkToHost64(uint64_t net64) {
    return be64toh(net64);
}

inline uint16_t networkToHost16(uint64_t net16) {
    return ntohs(net16);
}

void toIpPort(char* buf, size_t size,
                      const struct sockaddr* addr);

void toIp(char* buf, size_t size,
                  const struct sockaddr* addr);

void fromIpPort(const char* ip, uint16_t port,
                        struct sockaddr_in* addr);
void fromIpPort(const char* ip, uint16_t port,
                        struct sockaddr_in6* addr);

int createNonblockingOrDie(sa_family_t family);
void bindOrDie(int sockfd, const struct sockaddr_in& addr);
void listenOrDie(int sockfd);
int accept(int sockfd, struct sockaddr_in* addr);
void close(int sockfd);
void shutdownWrite(int sockfd);

void toHostPort(char* buf, const size_t size, const struct sockaddr_in& addr);
void fromHostPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
struct sockaddr_in6 getLocalAddr(int sockfd);
struct sockaddr_in6 getPeerAddr(int sockfd);
int  connect(int sockfd, const struct sockaddr* addr);
const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr);

int getSocketError(int sockfd);
bool isSelfConnect(int sockfd);
} // namespace sockets
} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_SOCKETSOPS_H
