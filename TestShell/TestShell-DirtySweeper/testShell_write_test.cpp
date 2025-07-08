#include "gmock/gmock.h"
#include "testShell.cpp"
#include <string>
using std::string;
using namespace testing;

class TestShellWriteTest : public Test {
public:
    NiceMock< SSDMock> ssdMock;
    TestShell sut{ &ssdMock };
    const int VALID_LBA = 0;
    const int INVALID_LBA = 100;
    const string VALID_DATA = "0x12345678";
    const string INVALID_DATA = "AHAHHAHAA";
};

TEST_F(TestShellWriteTest, Write)
{
    const string actual = "[Write] Done";
    EXPECT_CALL(ssdMock, write(VALID_LBA, VALID_DATA))
        .Times(1);

    string result = sut.write(VALID_LBA, VALID_DATA);

    EXPECT_EQ(actual, result);
}

TEST_F(TestShellWriteTest, WriteFailWithInvalidLBA)
{
    const string actual = "[Write] ERROR";
    EXPECT_CALL(ssdMock, write(INVALID_LBA, VALID_DATA))
        .Times(1)
        .WillOnce(Return());
    EXPECT_CALL(ssdMock, getResult())
        .Times(1)
        .WillOnce(Return("ERROR"));

    string result = sut.write(INVALID_LBA, VALID_DATA);

    EXPECT_EQ(actual, result);
}

TEST_F(TestShellWriteTest, WriteFailWithInvalidData)
{
    const string actual = "[Write] ERROR";
    EXPECT_CALL(ssdMock, write(INVALID_LBA, INVALID_DATA))
        .Times(1)
        .WillOnce(Return());
     EXPECT_CALL(ssdMock, getResult())
        .Times(1)
        .WillOnce(Return("ERROR"));

    string result = sut.write(INVALID_LBA, INVALID_DATA);

    EXPECT_EQ(actual, result);
}


