#include "gmock/gmock.h"
#include "testShell.cpp"

class MockCommandExecutor : public CommandExecutor {
public:
    MOCK_METHOD(std::string, read, (), (override));
    MOCK_METHOD(std::string, write, (), (override));
    MOCK_METHOD(std::string, exit, (), (override));
    MOCK_METHOD(std::string, help, (), (override));
    MOCK_METHOD(std::string, fullRead, (), (override));
    MOCK_METHOD(std::string, fullWrite, (), (override));
    MOCK_METHOD(std::string, testScript, (), (override));
};

TEST(TestShellTest, ValidCommand_Read) {
    MockCommandExecutor mock;
    TestShell shell;
	shell.setExecutor(&mock);

    EXPECT_CALL(mock, read()).WillOnce(::testing::Return("READ EXECUTED"));

    std::string result = shell.processCommand("read");
    EXPECT_EQ(result, "READ EXECUTED");
}

TEST(TestShellTest, ValidCommand_Help) {
    MockCommandExecutor mock;
    TestShell shell;
    shell.setExecutor(&mock);

    EXPECT_CALL(mock, help()).WillOnce(::testing::Return("HELP EXECUTED"));

    std::string result = shell.processCommand("help");
    EXPECT_EQ(result, "HELP EXECUTED");
}

TEST(TestShellTest, InvalidCommand_ShouldReturnInvalid) {
    MockCommandExecutor mock;
    TestShell shell;
    shell.setExecutor(&mock);

    std::string result = shell.processCommand("invalid_cmd");
    EXPECT_EQ(result, "INVALID COMMAND");
}