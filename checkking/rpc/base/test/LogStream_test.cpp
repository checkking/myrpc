#include "LogStream.h"
#include <gtest/gtest.h>

namespace checkking {
namespace rpc {
TEST(LogStreamTest, case_fixed_buffer) {
    FixedBuffer<1024> buffer;
    EXPECT_EQ(buffer.avial(), 1024);
    char a[] = "123";
    buffer.append(a, sizeof a);
    EXPECT_EQ(buffer.length(), sizeof a);
    EXPECT_STREQ(buffer.str().c_str(), "123");
    buffer.reset();
    EXPECT_EQ(buffer.avial(), 1024);
    buffer.bzero();
    char b[1026] = {'b'};
    buffer.append(b, sizeof b);
    EXPECT_EQ(buffer.avial(), 1024);
    EXPECT_EQ(buffer.length(), 0);
    buffer.append(b, 1023);
    EXPECT_EQ(buffer.length(), 1023);
    EXPECT_EQ(buffer.avial(), 1);
}

TEST(LogStreamTest, case_log_stream) {
    LogStream stream;
    stream << 1;
    EXPECT_STREQ(stream.buffer().str().c_str(), "1");
    stream << "abc";
    EXPECT_STREQ(stream.buffer().str().c_str(), "1abc");
    stream << 12.3;
    EXPECT_STREQ(stream.buffer().str().c_str(), "1abc12.3");
    stream << 12.000;
    EXPECT_STREQ(stream.buffer().str().c_str(), "1abc12.312");
    stream << "[DEBUG]" << "value:" << 1;
    EXPECT_STREQ(stream.buffer().str().c_str(), "1abc12.312[DEBUG]value:1");
    stream.resetBuffer();
    std::string str = std::string("abc");
    stream << str;
    EXPECT_STREQ(stream.buffer().str().c_str(), "abc");
}
} // namespace rpc
} // namespace checkking

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
