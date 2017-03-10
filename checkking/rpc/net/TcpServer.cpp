#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <string>
#include "SocketsOps.h"
#include "EventLoop.h"
#include "Logging.h"
#include "TcpConnection.h"
#include "TcpServer.h"
#include "EventLoopThreadPool.h"

namespace checkking {
namespace rpc {
TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr)
        : _loop(loop),
         _name(listenAddr.toHostPort()), 
         _acceptor(new Acceptor(loop, listenAddr)),
         _threadPool(new EventLoopThreadPool(loop)),
         _started(false), 
         _nextConnId(1) {
    _acceptor->setNewConnectionCallback(boost::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer() {

}

void TcpServer::start() {
    if (!_started) {
        _started = true;
        _threadPool->start();
    }
    if (!_acceptor->listenning()) {
        _loop->runInLoop(
                boost::bind(&Acceptor::listen, get_pointer(_acceptor)));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    _loop->assertInLoopThread();
    std::string connName = _name + (boost::format("#%d") % _nextConnId).str();
    ++_nextConnId;
    LOG_INFO << "TcpServer::newConnection [" << _name
            << "] - new connection [" << connName
            << "] from " << peerAddr.toHostPort();
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    EventLoop* ioLoop = _threadPool->getNextLoop();
    TcpConnectionPtr conn(
            new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    _connections[connName] = conn;
    conn->setConnectionCallback(_connectionCallback);
    conn->setMessageCallback(_messageCallback);
    conn->setCloseCallback(
            boost::bind(&TcpServer::removeConnection, this, _1));
    conn->connectEstablished();
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    _loop->runInLoop(boost::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    _loop->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnection [" << _name
           << "] - connection " << conn->name();
    size_t n = _connections.erase(conn->name());
    assert(n == 1); (void)n;
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        boost::bind(&TcpConnection::connectDestroyed, conn));
}
} // namespace rpc
} // namespace checkking
