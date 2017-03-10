#include "ThreadComm.h"

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <boost/weak_ptr.hpp>

namespace checkking {
namespace rpc {
namespace CurrentThread {
__thread pid_t t_cachedTid = 0;
__thread const char* t_threadName = "unknown";
pid_t tid() {
    if (t_cachedTid == 0) {
        t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
    }
    return t_cachedTid;
}

pid_t gettid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

void afterFork() {
    t_cachedTid = gettid();
    t_threadName = "main";
}
} // namespace CurrentThread

const char* CurrentThread::name() {
    return t_threadName;
}

bool CurrentThread::isMainThread() {
    return tid() == ::getpid();
}

struct ThreadData
{
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadFunc _func;
    std::string _name;
    boost::weak_ptr<pid_t> _wkTid;
    
    ThreadData(const ThreadFunc& func,
            const std::string& name,
            const boost::shared_ptr<pid_t>& tid)
        : _func(func), _name(name), _wkTid(tid) {}
        
    void runInThread() {
        pid_t tid = CurrentThread::tid();
        boost::shared_ptr<pid_t> ptid = _wkTid.lock();
        
        if (ptid) {
            *ptid = tid;
            ptid.reset();
        }
        
        CurrentThread::t_threadName = _name.c_str();
        _func();
        
        CurrentThread::t_threadName = "finished";
    }
};

void* startThread(void* obj) {
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    
    return NULL;
}

AtomicInt32 Thread::_numCreated;

Thread::Thread(const ThreadFunc& func, const std::string& n)
    : _started(false),
     _joined(false),
     _pthreadId(0),
     _tid(new pid_t(0)),
     _func(func),
     _name(n) {
    _numCreated.increment();
}

Thread::~Thread() {
    if (_started && _joined) {
        pthread_detach(_pthreadId);
    }
}

void Thread::start() {
    assert(!_started);
    _started = true;
    
    ThreadData* data = new ThreadData(_func, _name, _tid);
    if (pthread_create(&_pthreadId, NULL, &startThread, data)) {
        _started = false;
        delete data;
        abort();
    }
}

void Thread::join() {
    assert(_started);
    assert(!_joined);
    _joined = true;
    pthread_join(_pthreadId, NULL);
}

} // namespace rpc
} // namespace checkking
