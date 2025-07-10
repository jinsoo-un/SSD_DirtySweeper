#include "gmock/gmock.h"
#include "logger.h"
#include "ssd.h"
#include "testShell.h"
#include <string>

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
    static const string FULL_WRITE_SUCCESS_RESULT = "[Full Write] Done";
    static const string FULL_WRITE_FAIL_RESULT = "[Full Write] ERROR";
    static const string SSD_WRITE_DONE_VALUE = "";
    static const string SSD_WRITE_ERROR_VALUE = "ERROR";
}

class TestShellWriteTest : public Test {
public:
    NiceMock< SSDMock> ssdMock;
    NiceMock< MockTestShell> sut{ &ssdMock };
};

TEST_F(TestShellWriteTest, Write) {
    EXPECT_CALL(ssdMock, write(VALID_LBA, VALID_DATA))
        .Times(1);

    EXPECT_CALL(*static_cast<MockFileAccessor*>(MockFileAccessor::GetInstance()), readOutputFile())
        .Times(1)
        .WillOnce(Return(""));

    auto actual = sut.write(VALID_LBA, VALID_DATA);
    EXPECT_EQ(WRITE_SUCCESS_RESULT, actual);
}

TEST_F(TestShellWriteTest, WriteFailWithInvalidLBA) {
    EXPECT_CALL(ssdMock, write(INVALID_LBA, VALID_DATA))
        .Times(1)
        .WillOnce(Return());

    EXPECT_CALL(*static_cast<MockFileAccessor*>(MockFileAccessor::GetInstance()), readOutputFile())
        .Times(1)
        .WillOnce(Return(SSD_WRITE_ERROR_VALUE));

    auto actual = sut.write(INVALID_LBA, VALID_DATA);
    EXPECT_EQ(WRITE_FAIL_RESULT, actual);
}

TEST_F(TestShellWriteTest, WriteFailWithInvalidData) {
    EXPECT_CALL(ssdMock, write(VALID_LBA, INVALID_DATA))
        .Times(1)
        .WillOnce(Return());
    EXPECT_CALL(*static_cast<MockFileAccessor*>(MockFileAccessor::GetInstance()), readOutputFile())
        .Times(1)
        .WillOnce(Return(SSD_WRITE_ERROR_VALUE));

    auto actual = sut.write(VALID_LBA, INVALID_DATA);
    EXPECT_EQ(WRITE_FAIL_RESULT, actual);
}

TEST_F(TestShellWriteTest, FullWriteNormalCase) {
    EXPECT_CALL(ssdMock, write(_, VALID_DATA))
        .Times(100)
        .WillRepeatedly(Return());

    EXPECT_CALL(*static_cast<MockFileAccessor*>(MockFileAccessor::GetInstance()), readOutputFile())
        .Times(100)
        .WillRepeatedly(Return(SSD_WRITE_DONE_VALUE));

    auto actual = sut.fullWrite(VALID_DATA);
    EXPECT_EQ(FULL_WRITE_SUCCESS_RESULT, actual);
}

TEST_F(TestShellWriteTest, FullWriteFailWithInvalidData) {
    EXPECT_CALL(ssdMock, write(_, INVALID_DATA))
        .Times(1)
        .WillRepeatedly(Return());

    EXPECT_CALL(*static_cast<MockFileAccessor*>(MockFileAccessor::GetInstance()), readOutputFile())
        .Times(1)
        .WillRepeatedly(Return(SSD_WRITE_ERROR_VALUE));

    auto actual = sut.fullWrite(INVALID_DATA);
    EXPECT_EQ(FULL_WRITE_FAIL_RESULT, actual);
}
