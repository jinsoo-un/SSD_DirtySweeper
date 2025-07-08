#include "gmock/gmock.h"
#include "testShell.cpp"
using namespace testing;

TEST(ScriptTest, TC1) {
	EXPECT_EQ(1, 1);
}

TEST(WriteReadAging, CallTest) {
    testing::NiceMock<SSDMock> ssdMock;
    MockTestShell sut(&ssdMock);
    
    const string DATA = "0xAAAABBBB";

    EXPECT_CALL(ssdMock, write(0, DATA))
        .Times(TestShell::WRITE_READ_ITERATION);
    EXPECT_CALL(ssdMock, write(99, DATA))
        .Times(TestShell::WRITE_READ_ITERATION);
    EXPECT_CALL(ssdMock, read(0))
        .Times(TestShell::WRITE_READ_ITERATION);
    EXPECT_CALL(ssdMock, read(99))
        .Times(TestShell::WRITE_READ_ITERATION);

    EXPECT_CALL(sut, generateRandomHexString())
        .WillRepeatedly(Return("0xAAAABBBB"));

    EXPECT_CALL(sut, readOutputFile())
        .WillRepeatedly(Return("0xAAAABBBB"));

    sut.writeReadAging();
}

TEST(WriteReadAging, PassTest) {
    testing::NiceMock<SSDMock> ssdMock;
    MockTestShell sut(&ssdMock);

    const string DATA = "0xAAAABBBB";

    EXPECT_CALL(ssdMock, write(0, _))
        .Times(TestShell::WRITE_READ_ITERATION);
    EXPECT_CALL(ssdMock, write(99, _))
        .Times(TestShell::WRITE_READ_ITERATION);
    EXPECT_CALL(ssdMock, read(0))
        .Times(TestShell::WRITE_READ_ITERATION);
    EXPECT_CALL(ssdMock, read(99))
        .Times(TestShell::WRITE_READ_ITERATION);

    EXPECT_CALL(sut, generateRandomHexString())
        .WillRepeatedly(Return("0xAAAABBBB"));

    EXPECT_CALL(sut, readOutputFile())
        .WillRepeatedly(Return("0xAAAABBBB"));

    testing::internal::CaptureStdout();
    sut.writeReadAging();
    std::string output = testing::internal::GetCapturedStdout();
    cout << output;
    EXPECT_THAT(output, ::testing::HasSubstr("PASS"));
}

TEST(WriteReadAging, FailTest) {
    testing::NiceMock<SSDMock> ssdMock;
    MockTestShell sut(&ssdMock);

    const string DATA = "0xAAAABBBB";

    EXPECT_CALL(sut, generateRandomHexString())
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

TEST(PartialLBAWrite, PassCase) {
    testing::NiceMock<SSDMock> ssdMock;
    MockTestShell sut(&ssdMock);

    const string DATA = "0xAAAABBBB";
    EXPECT_CALL(ssdMock, write(_, _))
        .Times(5 * 30)
        .WillRepeatedly(Return());

    EXPECT_CALL(ssdMock, read(_))
        .Times(5 * 30)
        .WillRepeatedly(Return());

    EXPECT_CALL(sut, readOutputFile())
        .Times(5 * 30)
        .WillRepeatedly(Return(DATA));


    testing::internal::CaptureStdout();
    sut.partialLBAWrite();
    std::string output = testing::internal::GetCapturedStdout();
    cout << output;
    EXPECT_THAT(output, ::testing::HasSubstr("PASS"));
}

TEST(PartialLBAWrite, FailCase) {
    testing::NiceMock<SSDMock> ssdMock;
    MockTestShell sut(&ssdMock);

    const string DATA = "0xAAAABBBB";
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


    testing::internal::CaptureStdout();
    sut.partialLBAWrite();
    std::string output = testing::internal::GetCapturedStdout();
    cout << output;
    EXPECT_THAT(output, ::testing::HasSubstr("FAIL\n"));
}
