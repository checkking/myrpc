#ifndef CHECKKING_RPC_NET_TIMER_H
#define CHECKKING_RPC_NET_TIMER_H

#include <boost/noncopyable.hpp>

#include "Atomic.h"
#include "Timestamp.h"
#include "Callbacks.h"

namespace checkking {
namespace rpc {
class Timer : boost::noncopyable {
public:
    Timer(const TimerCallback& cb, Timestamp when, double interval)
        : _callback(cb),
        _expiration(when),
        _interval(interval),
        _repeat(_interval > 0.0),
       _sequence(_s_numCreated.incrementAndGet()) {}

    void run() const {
        _callback();
    }

    Timestamp expiration() const {
        return _expiration;
    }

    bool repeat() const {
        return _repeat;
    }

    int64_t sequence() const {
        return _sequence;
    }

    void restart(Timestamp now);

    static int64_t numCreated() {
        return _s_numCreated.get();
    }

private:
    const TimerCallback _callback;
    Timestamp _expiration;
    const double _interval;
    const bool _repeat;
    const int64_t _sequence;

    static AtomicInt64 _s_numCreated;
}; // class Timer
} // namespace rpc
} // namespace checkking
#endif // CHECKKING_RPC_NET_TIMER_H
