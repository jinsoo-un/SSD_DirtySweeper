#include "gmock/gmock.h"
#include "ssd.h"
#include "testShell.h"

using namespace testing;

class FlushTestFixture : public Test {
public:
    NiceMock<SSDMock> ssdMock;
    MockTestShell testShell{ &ssdMock };
};

TEST_F(FlushTestFixture, FlushSuccess) {
    EXPECT_CALL(ssdMock, flushSsdBuffer())
        .Times(1);

    EXPECT_CALL(testShell, readOutputFile())
        .WillRepeatedly(Return(""));

    testing::internal::CaptureStdout();
    testShell.flushSsdBuffer();
    string output = testing::internal::GetCapturedStdout();
    cout << output;
    EXPECT_THAT(output, ::testing::HasSubstr("[Flush] Done"));
}

TEST_F(FlushTestFixture, FlushFail) {
    EXPECT_CALL(ssdMock, flushSsdBuffer())
        .Times(1);

    EXPECT_CALL(testShell, readOutputFile())
        .WillRepeatedly(Return("ERROR"));

    testing::internal::CaptureStdout();
    testShell.flushSsdBuffer();
    string output = testing::internal::GetCapturedStdout();
    cout << output;
    EXPECT_THAT(output, ::testing::HasSubstr("[Flush] ERROR"));
}