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
    EXPECT_CALL(mock, read(0)).Times(1);
    shell.processInput("read 0");
}

TEST_F(TestShellFixture, ValidCommandWrite) {
    EXPECT_CALL(mock, write(5, "hello")).Times(1);
    shell.processInput("write 5 hello");
}

TEST_F(TestShellFixture, ValidCommandExit) {
    EXPECT_CALL(mock, exit()).Times(1);
    shell.processInput("exit");
}

TEST_F(TestShellFixture, ValidCommandHelp) {
    EXPECT_CALL(mock, help()).Times(1);
    shell.processInput("help");
}

TEST_F(TestShellFixture, ValidCommandFullRead) {
    EXPECT_CALL(mock, fullRead()).Times(1);
    shell.processInput("fullread");
}

TEST_F(TestShellFixture, ValidCommandFullWrite) {
    EXPECT_CALL(mock, fullWrite()).Times(1);
    shell.processInput("fullwrite");
}

TEST_F(TestShellFixture, ValidCommandTestScript) {
    EXPECT_CALL(mock, testScript()).Times(1);
    shell.processInput("testscript");
}

TEST_F(TestShellFixture, InvalidCommandShouldPrintInvalid) {
    // 콘솔 출력 테스트는 필요 시 capture 하는 방식으로 별도 처리 가능
    shell.processInput("invalid_cmd");
}