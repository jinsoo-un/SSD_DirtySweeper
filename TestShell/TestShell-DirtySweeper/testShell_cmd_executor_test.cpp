#include "gmock/gmock.h"
#include "testShell.cpp"

class MockCommandExecutor : public CommandExecutor {
public:
    MOCK_METHOD(void, read, (int lba), (override));
    MOCK_METHOD(void, write, (int lba, const std::string& data), (override));
    MOCK_METHOD(void, exit, (), (override));
    MOCK_METHOD(void, help, (), (override));
    MOCK_METHOD(void, fullRead, (), (override));
    MOCK_METHOD(void, fullWrite, (), (override));
    MOCK_METHOD(void, testScript, (), (override));
};

class TestShellFixture : public ::testing::Test {
protected:
    TestShellFixture() : shell() {
        shell.setExecutor(&mock);
    }
    MockCommandExecutor mock;
    TestShell shell;
};

TEST_F(TestShellFixture, ValidCommandRead) {
    int lba = 0;
    EXPECT_CALL(mock, read(lba)).Times(1);

    std::string result = shell.processInput("read 0");
    EXPECT_EQ(result, "READ DONE");
}

TEST_F(TestShellFixture, ValidCommand_Write) {
    int lba = 5;
    std::string data = "hello";

    EXPECT_CALL(mock, write(lba, data)).Times(1);

    std::string result = shell.processInput("write 5 hello");
    EXPECT_EQ(result, "WRITE DONE");
}

TEST_F(TestShellFixture, ValidCommand_Exit) {
    EXPECT_CALL(mock, exit()).Times(1);

    std::string result = shell.processInput("exit");
    EXPECT_EQ(result, "EXIT DONE");
}

TEST_F(TestShellFixture, ValidCommand_Help) {
    EXPECT_CALL(mock, help()).Times(1);

    std::string result = shell.processInput("help");
    EXPECT_EQ(result, "HELP DONE");
}

TEST_F(TestShellFixture, ValidCommand_FullRead) {
    EXPECT_CALL(mock, fullRead()).Times(1);

    std::string result = shell.processInput("fullread");
    EXPECT_EQ(result, "FULL READ DONE");
}

TEST_F(TestShellFixture, ValidCommand_FullWrite) {
    EXPECT_CALL(mock, fullWrite()).Times(1);

    std::string result = shell.processInput("fullwrite");
    EXPECT_EQ(result, "FULL WRITE DONE");
}

TEST_F(TestShellFixture, ValidCommand_TestScript) {
    EXPECT_CALL(mock, testScript()).Times(1);

    std::string result = shell.processInput("testscript");
    EXPECT_EQ(result, "TEST SCRIPT DONE");
}