#include "Acceptor.h"
#include "Logging.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include "InetAddress.h"

#include <boost/bind.hpp>

namespace checkking {
namespace rpc {
Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
        : _loop(loop), 
        _acceptSocket(sockets::createNonblockingOrDie()), 
        _acceptChannel(loop, _acceptSocket.fd()), 
        _newConnectionCallback(NULL),
        _listenning(false) {
    _acceptSocket.setReuseAddr(true);
    _acceptSocket.bindAddress(listenAddr);
    _acceptChannel.setReadCallback(
            boost::bind(&Acceptor::handleRead, this));
}

void Acceptor::listen() {
    _loop->assertInLoopThread();
    _acceptSocket.listen();
    _acceptChannel.enableReading();
    _listenning = true;
}

void Acceptor::handleRead() {
    _loop->assertInLoopThread();
    InetAddress peerAddr(0);
    int connfd = _acceptSocket.accept(&peerAddr);
    if (connfd >= 0) {
        if (_newConnectionCallback) {
            _newConnectionCallback(connfd, peerAddr);
        } else {
            sockets::close(connfd);
        }
    } else {
        // handle error.
    }
}
} // namespace rpc
} // namespace checkking
