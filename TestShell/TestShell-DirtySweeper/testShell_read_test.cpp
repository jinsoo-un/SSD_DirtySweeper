#include "gmock/gmock.h"
#include "logger.h"
#include "ssd.h"
#include "testShell.h"
#include "command.h"

using namespace testing;

class ReadTestFixture : public Test {
public:
    NiceMock<SSDMock> ssdMock;
    MockTestShell testShell{ &ssdMock };

    const string RANDOM_VALUE = "0xAAAABBBB";
    const string ERROR_RESULT = "ERROR";
    const string READ_ERROR_RESULT = "[Read] ERROR";
    const int RANDOM_LBA = 11;
    const int LBA_COUNT = 100;
};

TEST_F(ReadTestFixture, BasicRead) {
    EXPECT_CALL(ssdMock, read(_))
        .Times(1);

    EXPECT_CALL(*static_cast<MockFileAccessor*>(MockFileAccessor::GetInstance()), readOutputFile())
        .WillRepeatedly(Return(RANDOM_VALUE));

    ReadCommand(&ssdMock, RANDOM_LBA).execute();
}

TEST_F(ReadTestFixture, InvalidAddress) {
    EXPECT_CALL(ssdMock, read(_))
        .Times(0);

    EXPECT_CALL(*static_cast<MockFileAccessor*>(MockFileAccessor::GetInstance()), readOutputFile())
        .Times(0);

    auto output = ReadCommand(&ssdMock, 100).execute();
    EXPECT_EQ(READ_ERROR_RESULT, output);
}

TEST_F(ReadTestFixture, ReadSuccessTest) {
    EXPECT_CALL(ssdMock, read(_))
        .Times(1);

    EXPECT_CALL(*static_cast<MockFileAccessor*>(MockFileAccessor::GetInstance()), readOutputFile())
        .WillRepeatedly(Return(RANDOM_VALUE));

    ReadCommand(&ssdMock, RANDOM_LBA).execute();
    EXPECT_EQ(MockFileAccessor::GetInstance()->readOutputFile(), RANDOM_VALUE);
}

TEST_F(ReadTestFixture, ReadFailTest) {
    EXPECT_CALL(ssdMock, read(_))
        .Times(1);

    EXPECT_CALL(*static_cast<MockFileAccessor*>(MockFileAccessor::GetInstance()), readOutputFile())
        .WillRepeatedly(Return(ERROR_RESULT));

    ReadCommand(&ssdMock, 11).execute();
    EXPECT_EQ(MockFileAccessor::GetInstance()->readOutputFile(), ERROR_RESULT);
}

TEST_F(ReadTestFixture, TestIfReadCalled100Times) {
    EXPECT_CALL(ssdMock, read(_))
        .Times(LBA_COUNT);

    EXPECT_CALL(*static_cast<MockFileAccessor*>(MockFileAccessor::GetInstance()), readOutputFile())
        .WillRepeatedly(Return(RANDOM_VALUE));

    FullReadCommand(&ssdMock).execute();
}

TEST_F(ReadTestFixture, FulLReadFail) {
    EXPECT_CALL(ssdMock, read(_))
        .Times(4);

    EXPECT_CALL(*static_cast<MockFileAccessor*>(MockFileAccessor::GetInstance()), readOutputFile())
        .WillOnce(Return(RANDOM_VALUE))
        .WillOnce(Return(RANDOM_VALUE))
        .WillOnce(Return(RANDOM_VALUE))
        .WillRepeatedly(Return(ERROR_RESULT));

    FullReadCommand(&ssdMock).execute();
}
