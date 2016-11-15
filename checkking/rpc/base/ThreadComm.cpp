#include "ThreadComm.h"

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

namespace checkking {
namespace rpc {
namespace CurrentThread {
__thread pid_t t_cachedTid = 0;

pid_t tid() {
    if (t_cachedTid == 0) {
        t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
    }
    return t_cachedTid;
}

} // namespace CurrentThread
} // namespace rpc
} // namespace checkking
