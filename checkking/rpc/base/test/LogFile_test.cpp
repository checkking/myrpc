#include "LogFile.h"
#include "Logging.h"
#include <gtest/gtest.h>

namespace checkking {
namespace rpc {

boost::scoped_ptr<LogFile> g_logFile;

void fileOutputFunc(const char* msg, int len) {
    g_logFile->append(msg, len);
}

void fileFlushFunc() {
    g_logFile->flush();
}

TEST(LogFileTest, case_logfile) {
    g_logFile.reset(new LogFile(std::string("test"), 1024, true));
    Logger::setOutput(fileOutputFunc);
    Logger::setFlush(fileFlushFunc);
    std::string line = "133,debu,test";
    for (int i = 0; i < 4096; ++i) {
        LOG_INFO << line.c_str();
        usleep(1000);
    }
}

} // namespace rpc
} // namespace checkking

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
