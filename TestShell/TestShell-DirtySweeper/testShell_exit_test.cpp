#include "gmock/gmock.h"
#include "logger.h"
#include "ssd.h"
#include "testShell.h"

using namespace testing;

TEST(ExitTest, DefaultExitStatus) {
    SSDMock ssd;
    TestShell sut{ &ssd };

    EXPECT_FALSE(sut.isExit());
}

TEST(ExitTest, AfterExitCmd) {
    SSDMock ssd;
    TestShell sut{ &ssd };

    sut.exit();
    EXPECT_TRUE(sut.isExit());
}
