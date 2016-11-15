#ifndef CHECKKING_RPC_BASE_THREADCOMM_H
#define CHECKKING_RPC_BASE_THREADCOMM_H
#include <pthread.h>

namespace checkking {
namespace rpc {
namespace CurrentThread {
pid_t tid();
} // namespace CurrentThread
} // namespace rpc
} // namespace checkking
#endif // CHECKKING_RPC_BASE_THREADCOMM_H
