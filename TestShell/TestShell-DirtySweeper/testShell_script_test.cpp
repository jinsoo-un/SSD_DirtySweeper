#include "gmock/gmock.h"
#include "testShell.cpp"
using namespace testing;

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

class WriteReadAgingFixture : public Test {
public:
    WriteReadAgingFixture()
        : sut(&ssdMock), DATA("0xAAAABBBB") {
    }

    NiceMock<SSDMock> ssdMock;
    MockTestShell sut;

    const string DATA;

    string getWriteReadAgingResult() {
        testing::internal::CaptureStdout();
        sut.writeReadAging();
        string output = testing::internal::GetCapturedStdout();
        return output;
    }
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

TEST_F(FullWriteReadTest, FullWriteAndReadCompareShouldFail) {
    std::string evenData = EVEN_DATA;
    std::string oddData = ODD_DATA;
    const int ERROR_INJECTED_LBA = 13;

    for (int lbaIndex = START_LBA; lbaIndex <= END_LBA; ++lbaIndex) {
        if (lbaIndex > ERROR_INJECTED_LBA) break;
        std::string data = (lbaIndex / 5 % 2 == 0) ? evenData : oddData;
        EXPECT_CALL(ssd, write(lbaIndex, data)).Times(1);
        EXPECT_CALL(ssd, read(lbaIndex)).Times(1);
    }

    Sequence seq;
    for (int lbaIndex = START_LBA; lbaIndex <= END_LBA; ++lbaIndex) {
        std::string expected;
        if (lbaIndex > ERROR_INJECTED_LBA) break;

        if (lbaIndex == ERROR_INJECTED_LBA) {
            // 일부러 실패하도록 의도적으로 잘못된 값
            expected = "0xWRONGDATA";
        }
        else {
            expected = (lbaIndex / 5 % 2 == 0) ? "0xAAAABBBB" : "0xCCCCDDDD";
        }

        EXPECT_CALL(shell, readOutputFile())
            .InSequence(seq)
            .WillOnce(Return(expected));
    }

    // 실제 테스트
    shell.processInput("1_");
}

TEST_F(WriteReadAgingFixture, CallTest) {
    EXPECT_CALL(ssdMock, write(0, DATA))
        .Times(TestShell::WRITE_READ_ITERATION);
    EXPECT_CALL(ssdMock, write(99, DATA))
        .Times(TestShell::WRITE_READ_ITERATION);
    EXPECT_CALL(ssdMock, read(0))
        .Times(TestShell::WRITE_READ_ITERATION);
    EXPECT_CALL(ssdMock, read(99))
        .Times(TestShell::WRITE_READ_ITERATION);

    EXPECT_CALL(sut, getRandomHexString())
        .WillRepeatedly(Return(DATA));

    EXPECT_CALL(sut, readOutputFile())
        .WillRepeatedly(Return(DATA));

    sut.writeReadAging();
}

TEST_F(WriteReadAgingFixture, PassTest) {
    EXPECT_CALL(ssdMock, write(0, _))
        .Times(TestShell::WRITE_READ_ITERATION);
    EXPECT_CALL(ssdMock, write(99, _))
        .Times(TestShell::WRITE_READ_ITERATION);
    EXPECT_CALL(ssdMock, read(0))
        .Times(TestShell::WRITE_READ_ITERATION);
    EXPECT_CALL(ssdMock, read(99))
        .Times(TestShell::WRITE_READ_ITERATION);

    EXPECT_CALL(sut, getRandomHexString())
        .WillRepeatedly(Return(DATA));

    EXPECT_CALL(sut, readOutputFile())
        .WillRepeatedly(Return(DATA));

    EXPECT_THAT(getWriteReadAgingResult(), ::testing::HasSubstr("PASS"));
}

TEST_F(WriteReadAgingFixture, FailTest) {
    EXPECT_CALL(sut, getRandomHexString())
        .WillRepeatedly(Return(DATA));

    EXPECT_CALL(sut, readOutputFile())
        .WillOnce(Return(DATA))
        .WillOnce(Return(DATA))
        .WillOnce(Return(DATA))
        .WillOnce(Return(DATA))
        .WillOnce(Return(DATA))
        .WillRepeatedly(Return("ERROR"));

    testing::internal::CaptureStdout();
    sut.writeReadAging();
    std::string output = testing::internal::GetCapturedStdout();
    cout << output;
    EXPECT_THAT(output, ::testing::HasSubstr("FAIL"));
}

class PartialLBAWrite : public Test {
public:
    NiceMock<SSDMock> ssdMock;
    MockTestShell sut{ &ssdMock };

    const string DATA = "0xAAAABBBB";

    string getPartialLBAWriteResult() {
        testing::internal::CaptureStdout();
        sut.partialLBAWrite();
        return testing::internal::GetCapturedStdout();
    }
};

TEST_F(PartialLBAWrite, PassCase) {

    EXPECT_CALL(ssdMock, write(_, _))
        .Times(5 * 30)
        .WillRepeatedly(Return());

    EXPECT_CALL(ssdMock, read(_))
        .Times(5 * 30)
        .WillRepeatedly(Return());

    EXPECT_CALL(sut, readOutputFile())
        .Times(5 * 30)
        .WillRepeatedly(Return(DATA));

    EXPECT_THAT(getPartialLBAWriteResult(), ::testing::HasSubstr("PASS"));
}

TEST_F(PartialLBAWrite, FailCase) {
    const string MISMATCHED_DATA = "0xCCCCDDDD";

    EXPECT_CALL(ssdMock, write(_, _))
        .WillRepeatedly(Return());

    EXPECT_CALL(ssdMock, read(_))
        .WillRepeatedly(Return());

    EXPECT_CALL(sut, readOutputFile())
        .WillOnce(Return(DATA))
        .WillOnce(Return(DATA))
        .WillOnce(Return(DATA))
        .WillOnce(Return(DATA))
        .WillOnce(Return(MISMATCHED_DATA));

    EXPECT_THAT(getPartialLBAWriteResult(), ::testing::HasSubstr("FAIL\n"));
}

class EraseAndWriteAgingTest : public Test {
public:
    NiceMock<SSDMock> ssdMock;
    MockTestShell sut{ &ssdMock };

    const string SUCCESS_RESULT = "";
    const string ERROR_RESULT = "ERROR";

    string getEraseAndWriteAgingResult() {
        testing::internal::CaptureStdout();
        sut.eraseAndWriteAging();
        return testing::internal::GetCapturedStdout();
    }
};

TEST_F(EraseAndWriteAgingTest, PassCase) {

    const int MAX_AGING_CNT = 30;
    const int MAX_INTERNAL_LOOP_CNT = 49;
    const int WRITE_CNT_UNIT = 2;
    const int ERASE_CNT_UNIT = 1;

    const int MAX_WRITE_CNT = MAX_AGING_CNT * MAX_INTERNAL_LOOP_CNT * WRITE_CNT_UNIT;
    EXPECT_CALL(ssdMock, write(_, _))
        .Times(MAX_WRITE_CNT)
        .WillRepeatedly(Return());

    const int MAX_ERASE_CNT = MAX_AGING_CNT * MAX_INTERNAL_LOOP_CNT * ERASE_CNT_UNIT + 1;
    EXPECT_CALL(ssdMock, erase(_, _))
        .Times(MAX_ERASE_CNT)
        .WillRepeatedly(Return());

    const int MAX_READ_OUPUT_CNT = MAX_WRITE_CNT + MAX_ERASE_CNT;
    EXPECT_CALL(sut, readOutputFile())
        .Times(MAX_READ_OUPUT_CNT)
        .WillRepeatedly(Return(SUCCESS_RESULT));

    EXPECT_THAT(getEraseAndWriteAgingResult(), ::testing::HasSubstr("PASS"));
}
TEST_F(EraseAndWriteAgingTest, Fail) {
    EXPECT_CALL(ssdMock, write(_, _))
        .WillRepeatedly(Return());

    EXPECT_CALL(ssdMock, erase(_, _))
        .WillRepeatedly(Return());

    EXPECT_CALL(sut, readOutputFile())
        .WillOnce(Return(SUCCESS_RESULT))
        .WillOnce(Return(SUCCESS_RESULT))
        .WillOnce(Return(SUCCESS_RESULT))
        .WillRepeatedly(Return(ERROR_RESULT));

    EXPECT_THAT(getEraseAndWriteAgingResult(), ::testing::HasSubstr("FAIL"));
}

