#include <stdio.h>
#include <unistd.h>
#include <string>
#include <boost/bind.hpp>
#include "ThreadComm.h"

namespace checkking {
namespace rpc {
void threadFunc1() {
    printf("tid=%d\n", CurrentThread::tid());
}

void threadFunc2(int x) {
    printf("tid=%d, x=%d\n", CurrentThread::tid(), x);
}

void threadFunc3() {
    printf("tid=%d\n", CurrentThread::tid());
    timespec t = { 1, 0 };
    nanosleep(&t, NULL);
}

class Foo {
public:
    explicit Foo(double x) : _x(x) {
    }
    void func() {
        printf("tid=%d, Foo::x=%f\n", CurrentThread::tid(), _x);
    }
    void func2(const std::string& text) {
        printf("tid=%d, Foo::_x=%f, text=%s\n", CurrentThread::tid(), _x, text.c_str());
    }
private:
    double _x;

}; // calss Foo

} // namespace rpc
} // namespace checkking

int main(int argc, char **argv)
{
    using namespace checkking::rpc;
    Thread t1(threadFunc1);
    t1.start();
    t1.join();
    Thread t2(boost::bind(threadFunc2, 3));
    t2.start();
    t2.join();
    Foo foo(3.0);
    Thread t3(boost::bind(&Foo::func, &foo));
    t3.start();
    t3.join();
    return 0;
}
