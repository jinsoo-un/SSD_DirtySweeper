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

    std::string result = shell.executeCommand("read");
    EXPECT_EQ(result, "READ EXECUTED");
}

TEST_F(TestShellFixture, ValidCommand_Write) {
    EXPECT_CALL(mock, write()).WillOnce(::testing::Return("WRITE EXECUTED"));

    std::string result = shell.executeCommand("write");
    EXPECT_EQ(result, "WRITE EXECUTED");
}

TEST_F(TestShellFixture, ValidCommand_Exit) {
    EXPECT_CALL(mock, exit()).WillOnce(::testing::Return("EXIT EXECUTED"));

    std::string result = shell.executeCommand("exit");
    EXPECT_EQ(result, "EXIT EXECUTED");
}

TEST_F(TestShellFixture, ValidCommand_Help) {
    EXPECT_CALL(mock, help()).WillOnce(::testing::Return("HELP EXECUTED"));

    std::string result = shell.executeCommand("help");
    EXPECT_EQ(result, "HELP EXECUTED");
}

TEST_F(TestShellFixture, ValidCommand_FullRead) {
    EXPECT_CALL(mock, fullRead()).WillOnce(::testing::Return("FULL READ EXECUTED"));

    std::string result = shell.executeCommand("fullread");
    EXPECT_EQ(result, "FULL READ EXECUTED");
}

TEST_F(TestShellFixture, ValidCommand_FullWrite) {
    EXPECT_CALL(mock, fullWrite()).WillOnce(::testing::Return("FULL WRITE EXECUTED"));

    std::string result = shell.executeCommand("fullwrite");
    EXPECT_EQ(result, "FULL WRITE EXECUTED");
}

TEST_F(TestShellFixture, ValidCommand_TestScript) {
    EXPECT_CALL(mock, testScript()).WillOnce(::testing::Return("TEST SCRIPT EXECUTED"));

    std::string result = shell.executeCommand("testscript");
    EXPECT_EQ(result, "TEST SCRIPT EXECUTED");
}

TEST_F(TestShellFixture, InvalidCommand_ShouldReturnInvalid) {
    std::string result = shell.executeCommand("invalid_cmd");
    EXPECT_EQ(result, "INVALID COMMAND");
}