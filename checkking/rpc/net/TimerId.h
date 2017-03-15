#ifndef CHECKKING_RPC_NET_TIMERID_H
#define CHECKKING_RPC_NET_TIMERID_H

namespace checkking {
namespace rpc {
class Timer;

class TimerId {
public:
    TimerId(): _timer(NULL), _sequence(0) {}
    TimerId(Timer* timer, int64_t seq): _timer(timer), _sequence(seq) {}

    friend class TimerQueue;

private:
    Timer* _timer;
    int64_t _sequence;
}; // class TimerId
} // namespace rpc
} // namespace checkking

#endif // CHECKKING_RPC_NET_TIMERID_H
