#include "gmock/gmock.h"
#include "logger.h"
#include "ssd.h"
#include "testShell.h"
#include "command.h"

using namespace testing;

TEST(ExitTest, DefaultExitStatus) {
    SSDMock ssd;
    TestShell sut{ &ssd };

    EXPECT_FALSE(CommandFactory::isExit());
}

TEST(ExitTest, AfterExitCmd) {
    SSDMock ssd;
    TestShell sut{ &ssd };

    ExitCommand(&ssd).execute();
    EXPECT_TRUE(CommandFactory::isExit());
}
