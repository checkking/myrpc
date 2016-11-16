#include "EventLoop.h"
#include <sys/timerfd.h>
#include <gtest/gtest.h>
#include "Logging.h"
#include "Channel.h"

namespace checkking {
namespace rpc {

EventLoop g_loop;

void timeout() {
    LOG_INFO << "Time out!";
    g_loop.quit();
}

TEST(EventLoopTest, case_event_loop) {

    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Channel channel(&g_loop, timerfd);
    channel.setReadCallback(timeout);
    channel.enableReading();

    struct itimerspec howlong;
    bzero(&howlong, sizeof howlong);
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &howlong, NULL);
    g_loop.loop();
    ::close(timerfd);
}
} // namespace rpc
} // namespace checkking

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
