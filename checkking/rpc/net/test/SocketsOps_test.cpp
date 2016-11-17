#include "SocketsOps.h"
#include <gtest/gtest.h>

namespace checkking {
namespace rpc {
TEST(ScoketsOpsTest, case_socket_ops) {
    char ip[] = "180.76.146.17";
    uint16_t port = 8000;
    struct sockaddr_in addr;
    bzero(&addr, sizeof addr);
    sockets::fromHostPort(ip, port, &addr);

    char addrStr2[19];
    bzero(addrStr2, sizeof addrStr2);
    sockets::toHostPort(addrStr2, 19, addr);
    EXPECT_STREQ("180.76.146,17:8000", addrStr2);
}
} // namespace rpc
} // namespace checkking

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
