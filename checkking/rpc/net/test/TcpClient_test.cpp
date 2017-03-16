#include "EventLoop.h"
#include "InetAddress.h"
#include "Logging.h"
#include <boost/bind.hpp>
#include <utility>
#include <stdio.h>
#include <unistd.h>

#include "TcpClient.h"

namespace checkking {
namespace rpc {
std::string message = "Hello\n";

void onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->name().c_str(),
           conn->peerAddress().toHostPort().c_str());
    conn->send(message);
  }
  else
  {
    printf("onConnection(): connection [%s] is down\n",
           conn->name().c_str());
  }
}

void onMessage(const TcpConnectionPtr& conn,
               Buffer* buf,
               Timestamp receiveTime)
{
  printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
         buf->readableBytes(),
         conn->name().c_str(),
         receiveTime.toFormattedString().c_str());

  printf("onMessage(): [%s]\n", buf->retrieveAsString().c_str());
}

} // namespace rpc
} // namespace checkking

int main(int argc, char **argv)
{
    using namespace checkking::rpc;
    EventLoop loop;
    InetAddress addr("127.0.0.1", 9981);
    TcpClient client(&loop, addr);
    client.setConnectionCallback(onConnection);
    client.setMessageCallback(onMessage);
    // client.enableRetry();
    client.connect();
    loop.loop();
    return 0;
}
