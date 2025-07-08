#include "gmock/gmock.h"
#include "testShell.cpp"
#include <string>
using std::string;
using namespace testing;

namespace {
    static const int START_LBA = 0;
    static const int END_LBA = 99;
    static const int VALID_LBA = 2;
    static const int INVALID_LBA = 100;
    static const string VALID_DATA = "0x12345678";
    static const string INVALID_DATA = "AHAHHAHAA";
    static const string WRITE_SUCCESS_RESULT = "[Write] Done";
    static const string WRITE_FAIL_RESULT = "[Write] ERROR";
}

class TestShellWriteTest : public Test {
public:
    NiceMock< SSDMock> ssdMock;
    NiceMock< MockTestShell> sut{ &ssdMock };
};

TEST_F(TestShellWriteTest, Write)
{
    EXPECT_CALL(ssdMock, write(VALID_LBA, VALID_DATA))
        .Times(1);

    EXPECT_CALL(sut, readOutputFile())
        .Times(1)
        .WillOnce(Return(""));
    string result = sut.write(VALID_LBA, VALID_DATA);

    EXPECT_EQ(WRITE_SUCCESS_RESULT, result);
}

TEST_F(TestShellWriteTest, WriteFailWithInvalidLBA)
{
    EXPECT_CALL(ssdMock, write(INVALID_LBA, VALID_DATA))
        .Times(1)
        .WillOnce(Return());

    EXPECT_CALL(sut, readOutputFile())
        .Times(1)
        .WillOnce(Return("ERROR"));

    string result = sut.write(INVALID_LBA, VALID_DATA);

    EXPECT_EQ(WRITE_FAIL_RESULT, result);
}

TEST_F(TestShellWriteTest, WriteFailWithInvalidData)
{
    EXPECT_CALL(ssdMock, write(INVALID_LBA, INVALID_DATA))
        .Times(1)
        .WillOnce(Return());
    EXPECT_CALL(sut, readOutputFile())
        .Times(1)
        .WillOnce(Return("ERROR"));

    string result = sut.write(INVALID_LBA, INVALID_DATA);

    EXPECT_EQ(WRITE_FAIL_RESULT, result);
}
TEST_F(TestShellWriteTest, FullWriteNormalCase)
{
    string actual = "";
    for (int lba = START_LBA; lba <= END_LBA; lba++) actual += WRITE_SUCCESS_RESULT + "\n";
    EXPECT_CALL(ssdMock, write(_, VALID_DATA))
        .Times(100)
        .WillRepeatedly(Return());

    EXPECT_CALL(sut, readOutputFile())
        .Times(100)
        .WillRepeatedly(Return(""));

    string result = sut.fullWrite(VALID_DATA);
    EXPECT_EQ(actual, result);
}
TEST_F(TestShellWriteTest, FullWriteFailWithInvalidData)
{
    EXPECT_CALL(ssdMock, write(_, INVALID_DATA))
        .Times(1)
        .WillRepeatedly(Return());

    EXPECT_CALL(sut, readOutputFile())
        .Times(1)
        .WillRepeatedly(Return("ERROR"));

    string result = sut.fullWrite(INVALID_DATA);
    EXPECT_EQ(WRITE_FAIL_RESULT, result);
}


