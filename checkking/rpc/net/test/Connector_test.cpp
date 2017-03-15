#include <stdio.h>
#include "EventLoop.h"
#include "Connector.h"

namespace checkking {
namespace rpc {

EventLoop* g_loop;

void connectCallback(int sockfd) {
    printf("Connected!\n");
    g_loop->quit();
}

} // namespace rpc
} // namespace checkking

int main(int argc, char **argv)
{
    using namespace checkking::rpc;
    EventLoop loop;
    g_loop = &loop;
    InetAddress addr("127.0.0.1", 9981);
    ConnectorPtr connector(new Connector(&loop, addr));
    connector->setNewConnectionCallback(&connectCallback);
    connector->start();

    loop.loop();
    return 0;
}
