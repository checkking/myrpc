#include "Acceptor.h"
#include <stdio.h>
#include "EventLoop.h"
#include "Logging.h"

void newConnectionCallback(int sockfd, const checkking::rpc::InetAddress& peerAddr) {
    using namespace checkking::rpc;
    printf("New connection: accepted a new conneciton from: %s\n", peerAddr.toHostPort().c_str());
    //LOG_INFO << "New connection: accepted a new connection from " << peerAddr.toHostPort().c_str();
    ::write(sockfd, "How are you?\n", 13);
    close(sockfd);
}

int main(int argc, char **argv)
{
    using namespace checkking::rpc;

    InetAddress listenAddr(8091);
    EventLoop loop;
    Acceptor acceptor(&loop, listenAddr);
    acceptor.setNewConnectionCallback(newConnectionCallback);
    acceptor.listen();
    loop.loop();

    return 0;
}
