#include "gmock/gmock.h"
#include "testShell.cpp"

using ::testing::_;
using ::testing::Return;
using ::testing::Sequence;

class FullWriteReadTest : public ::testing::Test {
public:
    const std::string EVEN_DATA = "0xAAAABBBB";
    const std::string ODD_DATA = "0xCCCCDDDD";
    const int START_LBA = 0;
    const int END_LBA = 99;
protected:
    SSDMock ssd;
    MockTestShell shell;

    FullWriteReadTest() : shell(&ssd) {}
};

TEST_F(FullWriteReadTest, FullWriteAndReadCompareShouldPass) {
    std::string evenData = EVEN_DATA;
    std::string oddData = ODD_DATA;

    for (int lbaIndex = START_LBA; lbaIndex <= END_LBA; ++lbaIndex) {
        std::string data = (lbaIndex / 5 % 2 == 0) ? evenData : oddData;
        EXPECT_CALL(ssd, write(lbaIndex, data)).Times(1);
        EXPECT_CALL(ssd, read(lbaIndex)).Times(1);
    }

    Sequence seq;
    for (int lbaIndex = START_LBA; lbaIndex <= END_LBA; ++lbaIndex) {
        std::string expected = (lbaIndex / 5 % 2 == 0) ? "0xAAAABBBB" : "0xCCCCDDDD";
        EXPECT_CALL(shell, readOutputFile())
            .InSequence(seq)
            .WillOnce(Return(expected));
    }

    shell.processInput("1_");
}