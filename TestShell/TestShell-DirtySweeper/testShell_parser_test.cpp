#include "gmock/gmock.h"
#include "testShell.cpp"

TEST(TestShellTest, ValidCommand_Read) {
    MockCommandExecutor mock;
    TestShell shell(&mock);

    EXPECT_CALL(mock, read()).WillOnce(::testing::Return("READ EXECUTED"));

    std::string result = shell.processCommand("read");
    EXPECT_EQ(result, "READ EXECUTED");
}

TEST(TestShellTest, ValidCommand_Help) {
    MockCommandExecutor mock;
    TestShell shell(&mock);

    EXPECT_CALL(mock, help()).WillOnce(::testing::Return("HELP EXECUTED"));

    std::string result = shell.processCommand("help");
    EXPECT_EQ(result, "HELP EXECUTED");
}

TEST(TestShellTest, InvalidCommand_ShouldReturnInvalid) {
    MockCommandExecutor mock;
    TestShell shell(&mock);

    std::string result = shell.processCommand("invalid_cmd");
    EXPECT_EQ(result, "INVALID COMMAND");
}