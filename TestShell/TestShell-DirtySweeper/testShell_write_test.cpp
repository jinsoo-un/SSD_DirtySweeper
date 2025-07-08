#include "gmock/gmock.h"
#include "testShell.cpp"
#include <string>
using std::string;
using namespace testing;

class TestShellWriteTest : public Test {
public:
    NiceMock< SSDMock> ssdMock;
    TestShell sut{ &ssdMock };
};

TEST_F(TestShellWriteTest, Write)
{
    const int LBA = 1;
    const string DATA = "0x12345678";
    const string actual = "[Write] Done";

    EXPECT_CALL(ssdMock, write(LBA, DATA))
        .Times(1);
    string result = sut.write(LBA, DATA);
    EXPECT_EQ(actual, result);
}

TEST_F(TestShellWriteTest, WriteFailWithInvalidLBA)
{
    const int LBA = 100;
    const string DATA = "0x12345678";
    const string actual = "[Write] ERROR";
    EXPECT_CALL(ssdMock, write(LBA, DATA))
        .Times(1)
        .WillOnce(Return());

    EXPECT_CALL(ssdMock, getResult())
        .Times(1)
        .WillOnce(Return("ERROR"));
    string result = sut.write(LBA, DATA);
    EXPECT_EQ(actual, result);
}
TEST_F(TestShellWriteTest, WriteFailWithInvalidData)
{
    const int LBA = 100;
    const string DATA = "Its'n not hex data";
    const string actual = "[Write] ERROR";
    EXPECT_CALL(ssdMock, write(LBA, DATA))
        .Times(1)
        .WillOnce(Return());

    EXPECT_CALL(ssdMock, getResult())
        .Times(1)
        .WillOnce(Return("ERROR"));
    string result = sut.write(LBA, DATA);
    EXPECT_EQ(actual, result);
}
TEST_F(TestShellWriteTest, WriteFailWithInvalidOutput)
{
    const int LBA = 100;
    const string DATA = "Its'n not hex data";
    const string actual = "[Write] ERROR";
    EXPECT_CALL(ssdMock, write(LBA, DATA))
        .Times(1)
        .WillOnce(Return());

    EXPECT_CALL(ssdMock, getResult())
        .Times(1)
        .WillOnce(Return("NOT_DEFINED_RETURN_TYPE"));
    string result = sut.write(LBA, DATA);
    EXPECT_EQ(actual, result);
}

