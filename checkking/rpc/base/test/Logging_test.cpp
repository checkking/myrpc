#include "Logging.h"
#include <gtest/gtest.h>
namespace checkking {
namespace rpc {
TEST(LoggingTest, case_logging) {
    LOG_INFO << "test1";
    LOG_INFO << "test2" << ":" << 1;
    LOG_TRACE << "Trace";
    LOG_DEBUG << "DEBUG";
    // LOG_FATAL << "FATAL...";
}
} // namespace rpc
} // namespace checkking

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
