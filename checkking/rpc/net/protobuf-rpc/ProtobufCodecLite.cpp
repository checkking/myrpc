#include "ProtobufCodecLite.h"

#include <google/protobuf/message.h>
#include <zlib.h>

namespace {
int ProtobufVersionCheck() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    return 0;
}
int __attribute__ ((unused)) dummy = ProtobufVersionCheck();
}

namespace {
    const string kNoErrorStr = "NoError";
    const string kInvalidLengthStr = "InvalidLength";
    const string kCheckSumErrorStr = "CheckSumError";
    const string kInvalidNameLenStr = "InvalidNameLen";
    const string kUnknownMessageTypeStr = "UnknownMessageType";
    const string kParseErrorStr = "ParseError";
    const string kUnknownErrorStr = "UnknownError";
}

namespace checkking {
namespace rpc {

void ProtobufCodecLite::send(const TcpConnectionPtr& conn,
        const ::google::protobuf::Message& message) {
    /*
    Buffer* buf = conn.outputBuffer();
    fillEmptyBuffer(buf, message);
    */
    Buffer buf;
    fillEmptyBuffer(&buf, message);
    conn->send(&buf);
}

void ProtobufCodecLite::fillEmptyBuffer(Buffer* buf,
        const google::protobuf::Message& message) {
    assert(buf->readableBytes() == 0);
    buf->append(_tag);
    int byte_size = serializeToBuffer(message, buf);
    int32_t checkSum = checksum(buf->peek(), static_cast<int>(buf->readableBytes()));
    buf->appendInt32(checkSum);
    assert(buf->readableBytes() == tag_.size() + byte_size + kChecksumLen); (void) byte_size;
    int32_t len = sockets::hostToNetwork32(static_cast<int32_t>(buf->readableBytes()));
    buf->prepend(&len, sizeof len);
}

void ProtobufCodecLite::onMessage(const TcpConnectionPtr& conn,
        Buffer* buf,
        Timestamp receiveTime) {
    while (buf->readableBytes() >= static_cast<uint32_t>(kMinMessageLen+kHeaderLen)) {
        const int32_t len = buf->peekInt32();
        if (len > kMaxMessageLen || len < kMinMessageLen) {
            _errorCallback(conn, buf, receiveTime, kInvalidLength);
        } else if (buf->readableBytes() >= implicit_cast<size_t>(kHeaderLen+len)) {
            if (_rawCb && !_rawCb(conn, StringPiece(buf->peek(), kHeaderLen+len), receiveTime)) {
                buf->retrieve(kHeaderLen+len);
                continue;
            }
        }
        MessagePtr message(_prototype->New());
        ErrorCode errorCode = parse(buf->peek()+kHeaderLen, len, message.get());
        if (errorCode == kNoError) {
            _messageCallback(conn, message, receiveTime);
            buf->retrieve(kHeaderLen+len);
        } else {
            _errorCallback(conn, buf, receiveTime, errorCode);
            break;
        }
    } else {
        break;
    }
}

bool ProtobufCodecLite::parseFromBuffer(StringPiece buf, google::protobuf::Message* message) {
    return message->ParseFromArray(buf.data(), buf.size());
}

int ProtobufCodecLite::serializeToBuffer(const google::protobuf::Message& message, Buffer* buf) {
    int byte_size = message.ByteSize();
    buf->ensureWritableBytes(byte_size + kChecksumLen);
    uint8_t* start = reinterpret_cast<uint8_t*>(buf->beginWrite());
    uint8_t* end = message.SerializeWithCachedSizesToArray(start);
    if (end - start != byte_size) {
        ByteSizeConsistencyError(byte_size, message.ByteSize(), static_cast<int>(end - start));
    }
    buf->hasWritten(byte_size);
    return byte_size;
}

const string& ProtobufCodecLite::errorCodeToString(ErrorCode errorCode) {
    switch (errorCode) {
    case kNoError:
        return kNoErrorStr;
    case kInvalidLength:
        return kInvalidLengthStr;
    case kCheckSumError:
        return kCheckSumErrorStr;
    case kInvalidNameLen:
        return kInvalidNameLenStr;
    case kUnknownMessageType:
        return kUnknownMessageTypeStr;
    case kParseError:
        return kParseErrorStr;
    default:
        return kUnknownErrorStr;
    }
}

void ProtobufCodecLite::defaultErrorCallback(const TcpConnectionPtr& conn,
        Buffer* buf,
        Timestamp,
        ErrorCode errorCode) {
    LOG_ERROR << "ProtobufCodecLite::defaultErrorCallback - " << errorCodeToString(errorCode);
    if (conn && conn->connected()) {
        conn->shutdown();
    }
}

int32_t ProtobufCodecLite::asInt32(const char* buf) {
    int32_t be32 = 0;
    ::memcpy(&be32, buf, sizeof(be32));
    return sockets::networkToHost32(be32);
}

int32_t ProtobufCodecLite::checksum(const void* buf, int len) {
    return static_cast<int32_t>(
            ::adler32(1, static_cast<const Bytef*>(buf), len));
}

bool ProtobufCodecLite::validateChecksum(const char* buf, int len) {
    int32_t expectedCheckSum = asInt32(buf + len - kChecksumLen);
    int32_t checkSum = checksum(buf, len - kChecksumLen);
    return checkSum == expectedCheckSum;
}

ProtobufCodecLite::ErrorCode ProtobufCodecLite::parse(const char* buf,
        int len,
        ::google::protobuf::Message* message) {
    ErrorCode error = kNoError;
    if (validateChecksum(buf, len)) {
        if (memcmp(buf, _tag.data(), _tag.size()) == 0) {
            // parse from buffer
            const char* data = buf + _tag.size();
            int32_t dataLen = len - kChecksumLen - static_cast<int>(_tag.size());
            if (parseFromBuffer(StringPiece(data, dataLen), message)) {
                error = kNoError;
            } else {
                error = kParseError;
            }
        } else {
            error = kUnknownMessageType;
        }
    } else {
        error = kCheckSumError;
    }

    return error;
}
} // namespace rpc
} // namespace checkking
