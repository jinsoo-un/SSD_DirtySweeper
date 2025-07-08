#include "gmock/gmock.h"
#include "testShell.cpp"

using ::testing::_;
using ::testing::Return;
using ::testing::Sequence;

class FullWriteReadTest : public ::testing::Test {
protected:
    SSDMock ssd;
    MockTestShell shell;

    FullWriteReadTest() : shell(&ssd) {}
};

TEST_F(FullWriteReadTest, FullWriteAndReadCompareShouldPass) {
    std::string evenData = "0xAAAABBBB";
    std::string oddData = "0xCCCCDDDD";

    for (int i = 0; i <= 99; ++i) {
        std::string data = (i / 5 % 2 == 0) ? evenData : oddData;
        EXPECT_CALL(ssd, write(i, data)).Times(1);
        EXPECT_CALL(ssd, read(i)).Times(1);
    }

    Sequence seq;
    for (int i = 0; i <= 99; ++i) {
        std::string expected = (i / 5 % 2 == 0) ? "0xAAAABBBB" : "0xCCCCDDDD";
        EXPECT_CALL(shell, readOutputFile())
            .InSequence(seq)
            .WillOnce(Return(expected));
    }

    shell.processInput("1_");
}