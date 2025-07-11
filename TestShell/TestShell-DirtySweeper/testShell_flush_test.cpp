#include "gmock/gmock.h"
#include "ssd.h"
#include "testShell.h"
#include "command.h"

using namespace testing;

class FlushTestFixture : public Test {
public:
    NiceMock<SSDMock> ssdMock;
    MockTestShell testShell{ &ssdMock };
};

TEST_F(FlushTestFixture, FlushSuccess) {
    EXPECT_CALL(ssdMock, flushSsdBuffer())
        .Times(1);

    EXPECT_CALL(*static_cast<MockFileAccessor*>(MockFileAccessor::GetInstance()), readOutputFile())
        .WillRepeatedly(Return(""));

    string output = FlushCommand(&ssdMock).execute();
    EXPECT_EQ("[Flush] Done", output);
}

TEST_F(FlushTestFixture, FlushFail) {
    EXPECT_CALL(ssdMock, flushSsdBuffer())
        .Times(1);

    EXPECT_CALL(*static_cast<MockFileAccessor*>(MockFileAccessor::GetInstance()), readOutputFile())
        .WillRepeatedly(Return("ERROR"));

    string output = FlushCommand(&ssdMock).execute();
    EXPECT_EQ("[Flush] ERROR", output);
}

TEST_F(FlushTestFixture, FlushCallTest) {
    EXPECT_CALL(ssdMock, flushSsdBuffer())
        .Times(1);

    EXPECT_CALL(*static_cast<MockFileAccessor*>(MockFileAccessor::GetInstance()), readOutputFile())
        .WillRepeatedly(Return(""));

    testing::internal::CaptureStdout();
    testShell.processInput("flush");
    string output = testing::internal::GetCapturedStdout();
    cout << output;
    EXPECT_THAT(output, ::testing::HasSubstr("[Flush] Done"));
}