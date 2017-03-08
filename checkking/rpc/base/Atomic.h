#ifndef CHECKKING_RPC_BASE_ATOMIC_H
#define CHECKKING_RPC_BASE_ATOMIC_H
namespace checkking {
namespace rpc {
template<typename T>
class AtomicIntegerT : boost::noncopyable {
public:
    AtomicIntegerT() : _value(0) {}
    AtomicIntegerT(const AtomicIntegerT& that) : _value(that.get()) {}
    AtomicIntegerT& operator=(const AtomicIntegerT& that) {
        getAndSet(that.get());
        return *this;
    }

    T get() const {
        return __sync_val_compare_and_swap(const_cast<volatile T*>(&_value), 0, 0);
    }

    T getAndAdd(T x) {
        return __sync_fetch_and_add(&_value, x);
    }

    T addAndGet(T x) {
        return getAndAdd(x) + x;
    }

    T incrementAndGet() {
        return addAndGet(1);
    }

    void add(T x) {
        getAndAdd(x);
    }

    void increment() {
        incrementAndGet();
    }

    void decrement() {
        getAndAdd(-1);
    }

    T getAndSet(T newValue) {
        return __sync_lock_test_and_set(&_value, newValue);
    }

private:
    volatile T _value;
}; 

typedef AtomicIntegerT<int32_t> AtomicInt32;
typedef AtomicIntegerT<int64_t> AtomicInt64;
} // namespace rpc
} // namespace checkking
#endif // CHECKKING_RPC_BASE_ATOMIC_H
