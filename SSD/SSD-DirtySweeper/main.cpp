#include <iostream>
#include "ssd.cpp"
#include "gmock/gmock.h"
#include <thread>
#include <chrono>

using std::string;

class RealSSDTest : public ::testing::Test {
public:
    SSD* ssd = new RealSSD();
    string VALID_HEX_DATA = "0x1298CDEF";
    string INVALID_HEX_DATA = "0xABCDEFGH";
    string INITIAL_HEX_DATA = "0x00000000";
    static const int VALID_TEST_ADDRESS = 0;
    static const int VALID_TEST_ADDRESS_MAX = 99;
    static const int INVALID_TEST_ADDRESS = 100;
    static const int VALID_TEST_SIZE = 10;
    static const int INVALID_TEST_SIZE = 20;

	static const int DELAY_NANOS_FOR_WRITE = 1000000; // 10 millisecond

    void SetUp() override {
        ofstream file(FileNames::DATA_FILE);
        if (!file.is_open()) {
            cout << "Error opening file for setup test." << endl;
            return;
        }

        for (int i = MIN_ADDRESS; i < MAX_ADDRESS; i++) {
            file << i << "\t" << "0x00000000" << endl;
        }
        file.close();

        this_thread::sleep_for(chrono::nanoseconds(DELAY_NANOS_FOR_WRITE));
    }

    bool checkOutputFile(string expected) {
        ifstream fin(FileNames::OUTPUT_FILE);
        if (!fin.is_open()) {
            cout << "OUTPUT file open failed\n";
            return false;
        }

        string line;
        getline(fin, line);
        if (line != expected)
            return false;
        return true;
    }

    string buildCommand(string cmd, int lba, string data = "") {
        string cmdLine = cmd + " " + std::to_string(lba);
        if (cmd == "W" || cmd == "E") cmdLine = cmdLine + " " + data;
        return cmdLine;
    }
};


TEST_F(RealSSDTest, ReadTC_InitialValue)
{
    string cmd = buildCommand("R", 1);
    ssd->parseCommand(cmd);
    bool isPass = ssd->exec();
    EXPECT_EQ(true, isPass);
	EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(RealSSDTest, ReadTC_OutofRange)
{
    string cmd = buildCommand("R", 100);
	ssd->parseCommand(cmd);
    bool isPass = ssd->exec();
    EXPECT_EQ(false, isPass);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(RealSSDTest, ReadTC_ReturnData01)
{
    string cmd = buildCommand("R", 50);
    ssd->parseCommand(cmd);
    bool isPass = ssd->exec();
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(RealSSDTest, ReadTC_ReturnData02)
{
    string cmd = buildCommand("R", 30);
    ssd->parseCommand(cmd);
    bool isPass = ssd->exec();
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(RealSSDTest, ArgparseRead) {
    string cmd = buildCommand("R", 3);
    ssd->parseCommand(cmd);
    EXPECT_EQ(2, ssd->getArgCount());
    EXPECT_EQ("R", ssd->getOp());
    EXPECT_EQ(3, ssd->getAddr());
}

TEST_F(RealSSDTest, ArgparseWrite)
{
    string cmd = buildCommand("W", 3, VALID_HEX_DATA);
    ssd->parseCommand(cmd);
    EXPECT_EQ(3, ssd->getArgCount());
    EXPECT_EQ("W", ssd->getOp());
    EXPECT_EQ(3, ssd->getAddr());
    EXPECT_EQ("0x1298CDEF", ssd->getValue());
}

TEST_F(RealSSDTest, ArgparseErase)
{
    string cmd = buildCommand("E", 3, std::to_string(10));
    ssd->parseCommand(cmd);
    EXPECT_EQ(3, ssd->getArgCount());
    EXPECT_EQ("E", ssd->getOp());
    EXPECT_EQ(3, ssd->getAddr());
	EXPECT_EQ(10, ssd->getSize());
}

TEST_F(RealSSDTest, ArgparseInvalidOp)
{
    string cmd = buildCommand("S", 3);
    ssd->parseCommand(cmd);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(RealSSDTest, ArgparseInvalidAddr)
{
    string cmd = buildCommand("R", 300);
    ssd->parseCommand(cmd);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(RealSSDTest, ArgparseInvalidValue)
{
    string cmd = buildCommand("W", 3, INVALID_HEX_DATA);
    ssd->parseCommand(cmd);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(RealSSDTest, WritePass) {
    string cmd = buildCommand("W", VALID_TEST_ADDRESS, VALID_HEX_DATA);
    ssd->parseCommand(cmd);
	bool isPass = ssd->exec();
    EXPECT_TRUE(isPass);
}

TEST_F(RealSSDTest, WriteFailWithOutOfAddressRange) {
    string cmd = buildCommand("W", INVALID_TEST_ADDRESS, VALID_HEX_DATA);
    ssd->parseCommand(cmd);
    bool isPass = ssd->exec();
    EXPECT_FALSE(isPass);
}


TEST_F(RealSSDTest, WriteInvalidData00) {
    string cmd = buildCommand("W", VALID_TEST_ADDRESS, "0x1234567890000");
    ssd->parseCommand(cmd);
    bool isPass = ssd->exec();
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteInvalidData01) {
    string cmd = buildCommand("W", VALID_TEST_ADDRESS, "0x1234");
    ssd->parseCommand(cmd);
    bool isPass = ssd->exec();
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteInvalidData02) {
    string cmd = buildCommand("W", VALID_TEST_ADDRESS, "12345678");
    ssd->parseCommand(cmd);
    bool isPass = ssd->exec();
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteInvalidData03) {
    string cmd = buildCommand("W", VALID_TEST_ADDRESS, "0x1234ABzE");
    ssd->parseCommand(cmd);
    bool isPass = ssd->exec();
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteInvalidData04) {
    string cmd = buildCommand("W", VALID_TEST_ADDRESS, "0xA5CCH012");
    ssd->parseCommand(cmd);
    bool isPass = ssd->exec();
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteReadVerify00) {
    string cmd = buildCommand("R", VALID_TEST_ADDRESS);
    ssd->parseCommand(cmd);
    bool isPass = ssd->exec();

    this_thread::sleep_for(chrono::nanoseconds(DELAY_NANOS_FOR_WRITE));

    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));

    cmd = buildCommand("W", VALID_TEST_ADDRESS, VALID_HEX_DATA);
    ssd->parseCommand(cmd);
    isPass = ssd->exec();
    EXPECT_TRUE(isPass);

    this_thread::sleep_for(chrono::nanoseconds(DELAY_NANOS_FOR_WRITE));

    cmd = buildCommand("R", VALID_TEST_ADDRESS);
    ssd->parseCommand(cmd);
    isPass = ssd->exec();
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(VALID_HEX_DATA));
}

TEST_F(RealSSDTest, ErasePass) {
    string cmd = buildCommand("E", VALID_TEST_ADDRESS, to_string(VALID_TEST_SIZE));
	ssd->parseCommand(cmd);
    bool isPass = ssd->exec();
    EXPECT_TRUE(isPass);
}

TEST_F(RealSSDTest, EraseFailOutofRange) {
    string cmd = buildCommand("E", INVALID_TEST_ADDRESS, to_string(VALID_TEST_SIZE));
    ssd->parseCommand(cmd);
    bool isPass = ssd->exec();
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, EraseFailOutofRangeDestination) {
    string cmd = buildCommand("E", VALID_TEST_ADDRESS_MAX, to_string(VALID_TEST_SIZE));
    ssd->parseCommand(cmd);
    bool isPass = ssd->exec();
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, EraseFailExceedMaxSize) {
    string cmd = buildCommand("E", VALID_TEST_ADDRESS, to_string(INVALID_TEST_SIZE));
    ssd->parseCommand(cmd);
    bool isPass = ssd->exec();
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, EraseAndReadVerify) {
    ssd->parseCommand(buildCommand("W", VALID_TEST_ADDRESS, VALID_HEX_DATA));
    bool isPass = ssd->exec();
    EXPECT_TRUE(isPass);

    this_thread::sleep_for(chrono::nanoseconds(DELAY_NANOS_FOR_WRITE));

    ssd->parseCommand(buildCommand("E", VALID_TEST_ADDRESS, to_string(VALID_TEST_SIZE)));
    isPass = ssd->exec();
    EXPECT_TRUE(isPass);

    this_thread::sleep_for(chrono::nanoseconds(DELAY_NANOS_FOR_WRITE));

	ssd->parseCommand(buildCommand("R", VALID_TEST_ADDRESS));
    isPass = ssd->exec();
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

#ifdef NDEBUG
int main(int argc, char *argv[])
{
    SSD* ssd = new BufferedSSD();
    string inputLine;

    // skip ssd.exe myself
    for (int i = 1; i < argc; ++i) {
        if (i > 1) inputLine += " ";
        inputLine += argv[i];
    }

    if (!ssd->parseCommand(inputLine))
        return -1;
    ssd->exec();

    return 0;
}
#else
int main()
{
    ::testing::InitGoogleMock();
    return RUN_ALL_TESTS();
}
#endif