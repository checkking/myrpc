#ifndef CHECKKING_RPC_NET_TCPCLIENT_H
#define CHECKKING_RPC_NET_TCPCLIENT_H

#include <boost/noncopyable.hpp>
#include "Mutex.h"
#include "TcpConnection.h"

namespace checkking {
namespace rpc {
class Connector;
typedef boost::shared_ptr<Connector> ConnectorPtr;

class TcpClient : boost::noncopyable {
public:
    TcpClient(EventLoop* loop, const InetAddress& serverAdd);
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr connection() const {
        MutexLockGuard lock(_mutex);
        return _connection;
    }

    void setConnectionCallback(const ConnectionCallback& cb) {
        _connectionCallback = cb;
    }

    void setMessageCallback(const MessageCallback& cb) {
        _messageCallback = cb;
    }

private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr& conn);

    EventLoop* _loop;
    ConnectorPtr _connector;
    ConnectionCallback _connectionCallback;
    MessageCallback _messageCallback;
    bool _retry;
    bool _connect;

    int _nextConnId;
    mutable MutexLock _mutex;
    TcpConnectionPtr _connection;
}; // class TcpClient
} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_TCPCLIENT_H
