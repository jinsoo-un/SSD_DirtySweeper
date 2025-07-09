#include "gmock/gmock.h"
#include "testShell.cpp"

using ::testing::_;
using ::testing::Return;

class TestShellLogicTest : public ::testing::Test {
protected:
    SSDMock ssd;
    MockTestShell shell;

    TestShellLogicTest() : shell(&ssd) {}
};

TEST_F(TestShellLogicTest, ReadShouldCallSSDReadAndPrintResult) {
    EXPECT_CALL(ssd, read(1)).Times(1);
    EXPECT_CALL(shell, readOutputFile()).WillOnce(Return("DATA123"));
    shell.read(1);
}

TEST_F(TestShellLogicTest, ProcessInputReadShouldTriggerReadLogic) {
    EXPECT_CALL(ssd, read(1)).Times(1);
    EXPECT_CALL(shell, readOutputFile()).WillOnce(Return("DATA123"));
    shell.processInput("read 1");
}

TEST_F(TestShellLogicTest, FullReadShouldCallSSDReadForAllLBA) {
    EXPECT_CALL(ssd, read(_)).Times(100);
    EXPECT_CALL(shell, readOutputFile()).Times(100).WillRepeatedly(Return("DATA"));
    shell.fullRead();
}

TEST_F(TestShellLogicTest, WriteShouldCallSSDWriteAndReturnDoneOnSuccess) {
    EXPECT_CALL(ssd, write(5, "abc")).Times(1);
    EXPECT_CALL(shell, readOutputFile()).WillOnce(Return("OK"));

    testing::internal::CaptureStdout();
    shell.write(5, "abc");
    std::string output = testing::internal::GetCapturedStdout();
    cout << output;
    EXPECT_THAT(output, ::testing::HasSubstr("[Write] Done"));
}

TEST_F(TestShellLogicTest, WriteShouldReturnErrorOnFailure) {
    EXPECT_CALL(ssd, write(5, "abc")).Times(1);
    EXPECT_CALL(shell, readOutputFile()).WillOnce(Return("ERROR"));

    testing::internal::CaptureStdout();
    shell.write(5, "abc");
    std::string output = testing::internal::GetCapturedStdout();
    cout << output;
    EXPECT_THAT(output, ::testing::HasSubstr("[Write] ERROR"));
}

TEST_F(TestShellLogicTest, ProcessInputWriteShouldCallSSDWrite) {
    EXPECT_CALL(ssd, write(5, "hello")).Times(1);
    EXPECT_CALL(shell, readOutputFile()).WillOnce(Return("OK"));

    shell.processInput("write 5 hello");
}

TEST_F(TestShellLogicTest, ProcessInputHelpShouldPrintUsage) {
    testing::internal::CaptureStdout();
    shell.processInput("help");
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_THAT(output, ::testing::HasSubstr("read (LBA)"));
    EXPECT_THAT(output, ::testing::HasSubstr("write (LBA) (DATA)"));
}

TEST_F(TestShellLogicTest, InvalidCommandShouldPrintInvalid) {
    testing::internal::CaptureStdout();
    shell.processInput("invalid_cmd");
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_THAT(output, ::testing::HasSubstr("INVALID COMMAND"));
}
