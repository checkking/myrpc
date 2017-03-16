#include <boost/bind.hpp>
#include <stdio.h>  // snprintf

#include "Connector.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include "Logging.h"
#include "TcpClient.h"

namespace checkking {
namespace rpc {
namespace inner {
void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn) {
    loop->queueInLoop(boost::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr& connector) {
    //connector->
}
} // namespace inner

TcpClient::TcpClient(EventLoop* loop, const InetAddress& serverAddr)
        : _loop(loop), 
        _connector(new Connector(loop, serverAddr)),
        _retry(false),
        _connect(true),
        _nextConnId(1) {
    _connector->setNewConnectionCallback(
        boost::bind(&TcpClient::newConnection, this, _1));
}

TcpClient::~TcpClient() {
    TcpConnectionPtr conn;
    {
        MutexLockGuard lock(_mutex);
        conn = _connection;
    }
    if (conn) {
        CloseCallback cb = boost::bind(&inner::removeConnection, _loop, _1);
        _loop->runInLoop(boost::bind(&TcpConnection::setCloseCallback, conn, cb));
    } else {
        _connector->stop();
        _loop->runAfter(1, boost::bind(&inner::removeConnector, _connector));
    }
}

void TcpClient::connect() {
    _connect = true;
    _connector->start();
}

void TcpClient::disconnect() {
    _connect = false;
    {
        MutexLockGuard lock(_mutex);
        if (_connection) {
            _connection->shutdown();
        }
    }
}

void TcpClient::stop() {
    _connect = false;
    _connector->stop();
}

void TcpClient::newConnection(int sockfd) {
    _loop->assertInLoopThread();
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toHostPort().c_str(), _nextConnId);
    ++_nextConnId;
    std::string connName = buf;
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    TcpConnectionPtr conn(new TcpConnection(_loop, connName, sockfd, localAddr, peerAddr));
    conn->setConnectionCallback(_connectionCallback);
    conn->setMessageCallback(_messageCallback);
    conn->setCloseCallback(
            boost::bind(&TcpClient::removeConnection, this, _1)); // FIXME: unsafe
    {
        MutexLockGuard lock(_mutex);
        _connection = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
  _loop->assertInLoopThread();
  assert(_loop == conn->getLoop());

  {
    MutexLockGuard lock(_mutex);
    assert(_connection == conn);
    _connection.reset();
  }
  _loop->queueInLoop(boost::bind(&TcpConnection::connectDestroyed, conn));
  if (_retry && _connect)
  {
    LOG_INFO << "TcpClient::connect[" << this << "] - Reconnecting to "
             << _connector->serverAddress().toHostPort();
    _connector->restart();
  }
}

} // namespace rpc
} // namespace checkking
