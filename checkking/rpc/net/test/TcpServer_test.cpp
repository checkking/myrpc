#include "TcpServer.h"
#include "EventLoop.h"
#include "Logging.h"
#include "TcpConnection.h"
#include "InetAddress.h"

namespace checkking {
namespace rpc {

void onConnection(const checkking::rpc::TcpConnectionPtr& conn) {
    if (conn->connected()) {
        LOG_INFO << "New connection: accepted a new connection"
                << conn->name().c_str() << " from "
                << conn->peerAddress().toHostPort().c_str();
    } else {
        LOG_INFO << "onConnection(): connection: connection "
                << conn->name().c_str() << " is down.";
    }
}

void onMessage(const checkking::rpc::TcpConnectionPtr& conn, const char* data, size_t len) {
    LOG_INFO << "onMessage(): reviced" << len << " bytes from connection " << conn->name().c_str();
    std::string msg = std::string(data, len);
    conn->send(msg);
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
