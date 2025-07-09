#include "gmock/gmock.h"
#include "testShell.cpp"
#include <string>
using std::string;
using namespace testing;

namespace {
    const int MAX_SIZE = 100;
}

class TestShellEraseTest : public Test {
public:
    NiceMock< SSDMock> ssdMock;
    NiceMock< MockTestShell> sut{ &ssdMock };
    string getEraseResult(unsigned lba, unsigned int size) {
        testing::internal::CaptureStdout();
        sut.erase(lba, size);
        string output = testing::internal::GetCapturedStdout();
        return output;
    }
};

TEST_F(TestShellEraseTest, EraseForShort)
{
    const int startLBA = 0;
    const int size = 10;
    EXPECT_CALL(ssdMock, erase(startLBA, size))
        .Times(1)
        .WillRepeatedly(Return());

    EXPECT_CALL(sut, readOutputFile())
        .Times(1)
        .WillOnce(Return(""));

    auto actual = getEraseResult(startLBA, size);
    auto expected = ::testing::HasSubstr("[Erase] Done");
    EXPECT_THAT(actual, expected);
}

TEST_F(TestShellEraseTest, EraseForHalf)
{
    const int startLBA = 0;
    const int size = 50;
    EXPECT_CALL(ssdMock, erase(_, _))
        .Times(5)
        .WillRepeatedly(Return());

    EXPECT_CALL(sut, readOutputFile())
        .Times(5)
        .WillOnce(Return(""));

    auto actual = getEraseResult(startLBA, size);
    auto expected = ::testing::HasSubstr("[Erase] Done");
    EXPECT_THAT(actual, expected);
}

TEST_F(TestShellEraseTest, EraseForMax)
{
    const int startLBA = 0;
    const int size = 99;
    EXPECT_CALL(ssdMock, erase(_, _))
        .Times(10)
        .WillRepeatedly(Return());

    EXPECT_CALL(sut, readOutputFile())
        .Times(10)
        .WillOnce(Return(""));

    auto actual = getEraseResult(startLBA, size);
    auto expected = ::testing::HasSubstr("[Erase] Done");
    EXPECT_THAT(actual, expected);
}
TEST_F(TestShellEraseTest, EraseStartFromMiddle)
{
    const int startLBA = 50;
    const int size = 32;
    EXPECT_CALL(ssdMock, erase(_, _))
        .Times(4)
        .WillRepeatedly(Return());

    EXPECT_CALL(sut, readOutputFile())
        .Times(4)
        .WillOnce(Return(""));

    auto actual = getEraseResult(startLBA, size);
    auto expected = ::testing::HasSubstr("[Erase] Done");
    EXPECT_THAT(actual, expected);
}
TEST_F(TestShellEraseTest, EraseForLastSingle)
{
    const int startLBA = MAX_SIZE - 1;
    const int size = 1;
    EXPECT_CALL(ssdMock, erase(_, _))
        .Times(1)
        .WillRepeatedly(Return());

    EXPECT_CALL(sut, readOutputFile())
        .Times(1)
        .WillOnce(Return(""));

    auto actual = getEraseResult(startLBA, size);
    auto expected = ::testing::HasSubstr("[Erase] Done");
    EXPECT_THAT(actual, expected);
}

TEST_F(TestShellEraseTest, EraseForFirstSingle)
{
    const int startLBA = 0;
    const int size = 1;
    EXPECT_CALL(ssdMock, erase(_, _))
        .Times(1)
        .WillRepeatedly(Return());

    EXPECT_CALL(sut, readOutputFile())
        .Times(1)
        .WillOnce(Return(""));

    auto actual = getEraseResult(startLBA, size);
    auto expected = ::testing::HasSubstr("[Erase] Done");
    EXPECT_THAT(actual, expected);
}

TEST_F(TestShellEraseTest, ExceptionOverMaxSize)
{
    const int startLBA = 0;
    const int size = 1000;
    EXPECT_THROW(sut.erase(startLBA, size), std::exception);
}
TEST_F(TestShellEraseTest, ExceptionUnderMinSize)
{
    const int startLBA = 0;
    const int size = 0;
    EXPECT_THROW(sut.erase(startLBA, size), std::exception);
}
TEST_F(TestShellEraseTest, ExceptionOverMaxRange)
{
    const int startLBA = 99;
    const int size = 10;
    EXPECT_THROW(sut.erase(startLBA, size), std::exception);
}
