#include "gmock/gmock.h"
#include "testShell.cpp"
#include <string>
using std::string;
using namespace testing;


TEST(TestShellWriteTest, Write)
{
    NiceMock< SSDMock> ssdMock;
    TestShell sut(&ssdMock);
    const int LBA = 1;
    const string DATA = "0x12345678";
    const string actual = "[Write] Done";

    EXPECT_CALL(ssdMock, write(LBA, DATA))
        .Times(1);
    string result = sut.write(LBA, DATA);
    EXPECT_EQ(actual, result);
}
TEST(TestShellWriteTest, WriteFailWithInvalidLBA)
{
    NiceMock< SSDMock> ssdMock;
    TestShell sut(&ssdMock);
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

