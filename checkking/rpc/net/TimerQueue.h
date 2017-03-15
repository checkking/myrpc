#ifndef CHECKKING_RPC_NET_TIMERQUEUE_H
#define CHECKKING_RPC_NET_TIMERQUEUE_H

#include <boost/noncopyable.hpp>
#include <set>
#include <vector>

#include "Mutex.h"
#include "Timestamp.h"
#include "Callbacks.h"
#include "Channel.h"

namespace checkking {
namespace rpc {
class EventLoop;
class Timer;
class TimerId;

class TimerQueue : boost::noncopyable {
public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId addTimer(const TimerCallback& cb,
            Timestamp when,
            double interval);

    void cancel(TimerId timerId);

private:
    typedef std::pair<Timestamp, Timer*> Entry;
    typedef std::set<Entry> TimerList;
    typedef std::pair<Timer*, int64_t> ActiveTimer;
    typedef std::set<ActiveTimer> ActiveTimerSet;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);

    void handleRead();
    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);

    bool insert(Timer* timer);

    EventLoop* _loop;
    const int _timerfd;
    Channel _timerfdChannel;
    TimerList _timers;

    ActiveTimerSet _activeTimers;
    bool _callingExpiredTimers;
    ActiveTimerSet _cancelingTimers;
}; // class TimerQueue

} // namespace rpc
} // namespace checkking

#endif // CHECKKING_RPC_NET_TIMERQUEUE_H
