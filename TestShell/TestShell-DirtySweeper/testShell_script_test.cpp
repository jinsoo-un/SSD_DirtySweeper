#include "gmock/gmock.h"
#include "testShell.cpp"
using namespace testing;

TEST(ScriptTest, TC1) {
	EXPECT_EQ(1, 1);
}

TEST(WriteReadAging, CallTest) {
    testing::NiceMock<SSDMock> ssdMock;
    TestShell sut(&ssdMock);
    
    const string actual = "[Write] Done";
    const string DATA = "0xAAAABBBB";

    EXPECT_CALL(ssdMock, write(0, DATA))
        .Times(200);
    EXPECT_CALL(ssdMock, write(99, DATA))
        .Times(200);
    EXPECT_CALL(ssdMock, read(0))
        .Times(200);
    EXPECT_CALL(ssdMock, read(99))
        .Times(200);

    sut.writeReadAging();
}

TEST(WriteReadAging, PassTest) {
    testing::NiceMock<SSDMock> ssdMock;
    MockTestShell sut(&ssdMock);

    const string DATA = "0xAAAABBBB";

    EXPECT_CALL(ssdMock, write(0, DATA))
        .Times(200);
    EXPECT_CALL(ssdMock, write(99, DATA))
        .Times(200);
    EXPECT_CALL(ssdMock, read(0))
        .Times(200);
    EXPECT_CALL(ssdMock, read(99))
        .Times(200);
    EXPECT_CALL(sut, readOutputFile())
        .WillRepeatedly(Return("0xAAAABBBB"));

    testing::internal::CaptureStdout();
    sut.writeReadAging();
    std::string output = testing::internal::GetCapturedStdout();
    cout << output;
    EXPECT_THAT(output, ::testing::HasSubstr("PASS"));
}