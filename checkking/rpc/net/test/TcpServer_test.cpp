#include "TcpServer.h"
#include "EventLoop.h"
#include "Logging.h"
#include "TcpConnection.h"
#include "InetAddress.h"
#include "Timestamp.h"

namespace checkking {
namespace rpc {

void onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        LOG_INFO << "New connection: accepted a new connection"
                << conn->name().c_str() << " from "
                << conn->peerAddress().toHostPort().c_str();
    } else {
        LOG_INFO << "onConnection(): connection: connection "
                << conn->name().c_str() << " is down.";
    }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime) {
    LOG_INFO << "onMessage(): reviced" << buf->readableBytes() << " bytes from connection "
            << conn->name().c_str()
            << ", receiveTime:" << receiveTime.toFormattedString().c_str();
    conn->send(buf->retrieveAsString());
}
}
}

int main(int argc, char **argv)
{
    using namespace checkking::rpc;
    EventLoop loop;
    InetAddress listenAddr(9981);
    TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();
    loop.loop();

    return 0;
}
