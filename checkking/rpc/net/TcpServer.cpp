#include <boost/bind.hpp>
#include "TcpServer.h"

namespace checkking {
namespace rpc {
TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr)
        : _loop(loop),
         _name(listenAddr.toHostPort()), 
         _acceptor(new Acceptor(loop, listenAddr)),
         _started(false), 
         _nextConnId(1) {
    _acceptor->setNewConnectionCallback(boost::bind(&TcpServer::newConnection, this, _1, _2));
    // _acceptor->setNewConnectionCallback(&TcpServer::newConnection);
}

TcpServer::~TcpServer() {

}

} // namespace rpc
} // namespace checkking
