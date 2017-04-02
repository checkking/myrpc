#ifndef CHECKKING_RPC_NET_PROTO_PROTOBUFCODECLITE_H
#define CHECKKING_RPC_NET_PROTO_PROTOBUFCODECLITE_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include "StringPiece.h"
#include "Timestamp.h"
#include "Callbacks.h"

namespace google {
namespace protobuf {
class Message;
}
}

namespace checkking {
namespace rpc {
class Buffer;
class TcpConnection;
typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef boost::shared_ptr<google::protobuf::Message> MessagePtr;

class ProtobufCodecLite : boost::noncopyable {
public:
    const static int kHeaderLen = sizeof(int32_t);
    const static int kChecksumLen = sizeof(int32_t);
    const static int kMaxMessageLen = 64 * 1024 * 1024;
    enum ErrorCode {
        kNoError = 0,
        kInvalidLength,
        kCheckSumError,
        kInvalidNameLen,
        kUnknownMessageType,
        kParseError,
    };
    typedef boost::function<bool (const TcpConnectionPtr&,
            StringPiece,
            Timestamp)> RawMessageCallback;
    typedef boost::function<void (const TcpConnectionPtr&,
            const MessagePtr&,
            Timestamp)> ProtobufMessageCallback;
    typedef boost::function<void (const TcpConnectionPtr&,
            Buffer*,
            Timestamp,
            ErrorCode)> ErrorCallback;
    ProtobufCodecLite(const ::google::protobuf::Message* prototype,
            StringPiece tagArg,
            const ProtobufMessageCallback& messageCb,
            const RawMessageCallback& rawCb = RawMessageCallback(),
            const ErrorCallback& errorCb = defaultErrorCallback)
        : _prototype(prototype),
        _tag(tagArg.as_string()),
        _messageCallback(messageCb),
        _rawCb(rawCb),
        _errorCallback(errorCb),
        kMinMessageLen(tagArg.size() + kChecksumLen) {
    }

    virtual ~ProtobufCodecLite() { }

    const string& tag() const { return _tag; }

    void send(const TcpConnectionPtr& conn,
            const ::google::protobuf::Message& message);

    void onMessage(const TcpConnectionPtr& conn,
            Buffer* buf,
            Timestamp receiveTime);

    virtual bool parseFromBuffer(StringPiece buf, google::protobuf::Message* message);

    virtual int serializeToBuffer(const google::protobuf::Message& message, Buffer* buf);

    static const string& errorCodeToString(ErrorCode errorCode);

    ErrorCode parse(const char* buf, int len, ::google::protobuf::Message* message);
    void fillEmptyBuffer(muduo::net::Buffer* buf, const google::protobuf::Message& message);

    static int32_t checksum(const void* buf, int len);

    static bool validateChecksum(const char* buf, int len);
    static int32_t asInt32(const char* buf);
    static void defaultErrorCallback(const TcpConnectionPtr&,
            Buffer*,
            Timestamp,
            ErrorCode);
private:
    const ::google::protobuf::Message* _prototype;
    const string _tag;
    ProtobufMessageCallback _messageCallback;
    RawMessageCallback _rawCb;
    ErrorCallback _errorCallback;
    const int kMinMessageLen;
}; 

template<typename MSG, const char* TAG, typename CODEC=ProtobufCodecLite>  // TAG must be a variable with external linkage, not a string literal
class ProtobufCodecLiteT {
public:
    typedef boost::shared_ptr<MSG> ConcreteMessagePtr;
    typedef boost::function<void (const TcpConnectionPtr&,
            const ConcreteMessagePtr&,
            Timestamp)> ProtobufMessageCallback;
    typedef ProtobufCodecLite::RawMessageCallback RawMessageCallback;
    typedef ProtobufCodecLite::ErrorCallback ErrorCallback;

    explicit ProtobufCodecLiteT(const ProtobufMessageCallback& messageCb,
            const RawMessageCallback& rawCb = RawMessageCallback(),
            const ErrorCallback& errorCb = ProtobufCodecLite::defaultErrorCallback)
        : _messageCallback(messageCb),
        _codec(&MSG::default_instance(),
                TAG,
                boost::bind(&ProtobufCodecLiteT::onRpcMessage, this, _1, _2, _3),
                rawCb,
                errorCb) {
    }

    const string& tag() const { return _codec.tag(); }

    void send(const TcpConnectionPtr& conn,
            const MSG& message) {
        _codec.send(conn, message);
    }

    void onMessage(const TcpConnectionPtr& conn,
            Buffer* buf,
            Timestamp receiveTime) {
        _codec.onMessage(conn, buf, receiveTime);
    }
    void onRpcMessage(const TcpConnectionPtr& conn,
            const MessagePtr& message,
            Timestamp receiveTime) {
        messageCallback_(conn, down_pointer_cast<MSG>(message), receiveTime);
    }

    void fillEmptyBuffer(muduo::net::Buffer* buf, const MSG& message) {
        _codec.fillEmptyBuffer(buf, message);
    }

private:
    ProtobufMessageCallback _messageCallback;
    CODEC _codec;
};
} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_PROTO_PROTOBUFCODECLITE_H
