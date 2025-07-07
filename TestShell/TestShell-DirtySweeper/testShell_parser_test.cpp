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

class TestShellFixture : public ::testing::Test {
protected:
    TestShellFixture() : shell() {
        shell.setExecutor(&mock);
    }
    MockCommandExecutor mock;
    TestShell shell;
};

TEST_F(TestShellFixture, ValidCommand_Read) {
    EXPECT_CALL(mock, read()).WillOnce(::testing::Return("READ EXECUTED"));

    std::string result = shell.processCommand("read");
    EXPECT_EQ(result, "READ EXECUTED");
}

TEST_F(TestShellFixture, ValidCommand_Help) {
    EXPECT_CALL(mock, help()).WillOnce(::testing::Return("HELP EXECUTED"));

    std::string result = shell.processCommand("help");
    EXPECT_EQ(result, "HELP EXECUTED");
}

TEST_F(TestShellFixture, InvalidCommand_ShouldReturnInvalid) {
    std::string result = shell.processCommand("invalid_cmd");
    EXPECT_EQ(result, "INVALID COMMAND");
}