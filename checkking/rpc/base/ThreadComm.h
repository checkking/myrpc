#ifndef CHECKKING_RPC_BASE_THREADCOMM_H
#define CHECKKING_RPC_BASE_THREADCOMM_H
#include <pthread.h>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <pthread.h>
#include "Atomic.h"

namespace checkking {
namespace rpc {
namespace CurrentThread {
pid_t tid();
const char* name();
bool isMainThread();
} // namespace CurrentThread

class Thread: boost::noncopyable {
    public:
        typedef boost::function<void ()> ThreadFunc;
        explicit Thread(const ThreadFunc&, const std::string& name = std::string());
        ~Thread();

        pid_t tid() const {
            return *_tid;
        }
        void start();
        void join();

        std::string name() const {	
		    return _name;
		}
        bool started() const {
            return _started;
        }

    private:
        bool _started;
        bool _joined;
        pthread_t _pthreadId;
        boost::shared_ptr<pid_t> _tid;
        ThreadFunc _func;
        std::string _name;
        
        static AtomicInt32 _numCreated;
}; // class Thread

} // namespace rpc
} // namespace checkking
#endif // CHECKKING_RPC_BASE_THREADCOMM_H
