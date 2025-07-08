#include "gmock/gmock.h"
#include "testShell.cpp"
using namespace testing;

TEST(ScriptTest, TC1) {
	EXPECT_EQ(1, 1);
}

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

TEST_F(WriteReadAgingFixture, CallTest) {
    EXPECT_CALL(ssdMock, write(0, DATA))
        .Times(TestShell::WRITE_READ_ITERATION);
    EXPECT_CALL(ssdMock, write(99, DATA))
        .Times(TestShell::WRITE_READ_ITERATION);
    EXPECT_CALL(ssdMock, read(0))
        .Times(TestShell::WRITE_READ_ITERATION);
    EXPECT_CALL(ssdMock, read(99))
        .Times(TestShell::WRITE_READ_ITERATION);

    EXPECT_CALL(sut, generateRandomHexString())
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

    EXPECT_CALL(sut, generateRandomHexString())
        .WillRepeatedly(Return(DATA));

    EXPECT_CALL(sut, readOutputFile())
        .WillRepeatedly(Return(DATA));

    EXPECT_THAT(getWriteReadAgingResult(), ::testing::HasSubstr("PASS"));
}

TEST_F(WriteReadAgingFixture, FailTest) {
    EXPECT_CALL(sut, generateRandomHexString())
        .WillRepeatedly(Return(DATA));

    EXPECT_CALL(sut, readOutputFile())
        .WillOnce(Return(DATA))
        .WillOnce(Return(DATA))
        .WillOnce(Return(DATA))
        .WillOnce(Return(DATA))
        .WillOnce(Return(DATA))
        .WillRepeatedly(Return("ERROR"));

    EXPECT_THAT(getWriteReadAgingResult(), ::testing::HasSubstr("FAIL"));
}