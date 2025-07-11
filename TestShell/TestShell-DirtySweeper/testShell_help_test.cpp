#include "gmock/gmock.h"
#include "logger.h"
#include "ssd.h"
#include "testShell.h"
#include "command.h"

using namespace testing;

TEST(HelpTest, HelpCallTest) {
    SSDMock ssd;
    NiceMock<MockTestShell> mockShell{ &ssd };

    string output = HelpCommand(&ssd).execute();
    EXPECT_THAT(output, HasSubstr("read"));
    EXPECT_THAT(output, HasSubstr("write"));
    EXPECT_THAT(output, HasSubstr("fullread"));
    EXPECT_THAT(output, HasSubstr("fullwrite"));
    EXPECT_THAT(output, HasSubstr("help"));
    EXPECT_THAT(output, HasSubstr("exit"));
    EXPECT_THAT(output, HasSubstr("Team Members"));
}