#ifndef CHECKKING_RPC_NET_BUFFER_H
#define CHECKKING_RPC_NET_BUFFER_H

#include <vector>
#include <algorithm>
#include <string>

#include <assert.h>

namespace checkking {
namespace rpc {

class Buffer {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    Buffer(): _buffer(kCheapPrepend + kInitialSize),
            _readerIdx(kCheapPrepend),
            _writerIdx(kCheapPrepend) {
    }

    size_t readableBytes() const {
        return _writerIdx - _readerIdx;
    }

    size_t writableBytes() const {
        return _buffer.size() - _writerIdx;
    }

    size_t prependableBytes() const {
        return _readerIndex;
    }

    const char* peek() const {
        return begin() + _readerIndex;
    }

    void retrieve(size_t len) {
        assert(len <= readableBytes());
        _readerIdx += len;
    }

    void retrieveAll(const char* end) {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    std::string retrieveAsString() {
        std::string str(peek(), readableBytes());
        retrieveAll();
        return str;
    }

    void append(const std::string& str) {
        append(str.data(), str.length());
    }

    void append(const char* data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data+len, beginWrite());
        hasWritten(len);
    }

    void append(const void* data, size_t len) {
        append(static_cast<const char*>(data), len);
    }

    void ensureWritableBytes(size_t len) {
        if (writableBytes() < len) {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    char* beginWrite() {
        return begin() + _writerIndex;
    }

    const char* beginWrite() const {
        return begin() + _writerIndex;
    }

    void hasWritten(size_t len) {
        _writerIndex += len;
    }

    void prepend(const void* /*restrict*/ data, size_t len) {
        assert(len <= prependableBytes());
        _readerIndex -= len;
        const char* d = static_cast<const char*>(data);
        std::copy(d, d + len, begin() + _readerIndex);
    }

    void shrink(size_t reserve) {
        std::vector<char> buf(kCheapPrepend + readableBytes() + reserve);
        std::copy(peek(), peek() + readableBytes(), buf.begin() + kCheapPrepend);
        buf.swap(_buffer);
    }

    ssize_t readFd(int fd, int* savedErrno);

private:

    char* begin() {
        return &*_buffer.begin();
    }

    const char* begin() const {
        return &*_buffer.begin();
    }

    void makeSpace(size_t len) {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
            _buffer.resize(_writerIndex + len);
        } else {
            // move readable data to the front, make space inside buffer
            assert(kCheapPrepend < _readerIndex);
            size_t readable = readableBytes();
            std::copy(begin() + _readerIndex,
                    begin() + _writerIndex,
                    begin() + kCheapPrepend);
            assert(readable == readableBytes());
        }
    }

    std::vector<char> _buffer;
    size_t _readerIdx;
    size_t _writerIdx;
}; // class Buffer
} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_BUFFER_H
