#ifndef CHECKKING_RPC_NET_TCPSERVER_H
#define CHECKKING_RPC_NET_TCPSERVER_H

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <map>
#include "Callbacks.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"

namespace checkking {
namespace rpc {

class EventLoop;
class InetAddress;

class TcpServer : boost::noncopyable {
public:
    TcpServer(EventLoop* loop, const InetAddress& listenAddr);
    ~TcpServer();

    void start();

    void setConnectionCallback(const ConnectionCallback& cb) {
        _connectionCallback = cb;
    }

    void setMessageCallback(const MessageCallback& cb) {
        _messageCallback = cb;
    }

    void removeConnection(const TcpConnectionPtr& conn);

    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    void setThreadNum(int num) {
        assert(!_started);
        _threadPool->setThreadNum(num);
    }

private:
    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;
    void newConnection(int sockfd, const InetAddress& peerAddr);

    EventLoop* _loop;
    const std::string _name;
    boost::scoped_ptr<Acceptor> _acceptor;
    boost::scoped_ptr<EventLoopThreadPool> _threadPool;
    bool _started;
    int _nextConnId;
    ConnectionCallback _connectionCallback;
    MessageCallback _messageCallback;
    CloseCallback _closeCallback;
    ConnectionMap _connections;
}; // class TcpServer
} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_TCPSERVER_H
