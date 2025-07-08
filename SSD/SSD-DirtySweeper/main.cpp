#include <iostream>
#include "ssd.cpp"
#include "gmock/gmock.h"

using std::string;

class SSDTest : public ::testing::Test {
public:
    SSD ssd;
    ReadCommand readcmd;
    WriteCommand writecmd;
    string VALID_HEX_DATA = "0x1298CDEF";
    string INVALID_HEX_DATA = "0xABCDEFGH";
    string INITIAL_HEX_DATA = "0x00000000";
    static const int VALID_TEST_ADDRESS = 0;
    static const int INVALID_TEST_ADDRESS = 100;

    void SetUp() override {
        ssd.erase();
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

    string buildCommand(string rw, int lba, string data = "") {
        string cmdLine = rw + " " + std::to_string(lba);
        if (rw == "W") cmdLine = cmdLine + " " + data;
        return cmdLine;
    }
};


TEST_F(SSDTest, ReadTC_InitialValue)
{
    int lba_addr = 1;
    bool isPass;
    isPass = readcmd.run(lba_addr);
    EXPECT_EQ(true, isPass);
	EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(SSDTest, ReadTC_OutofRange)
{
    int lba_addr = 100;
    bool isPass;
    isPass = readcmd.run(lba_addr);
    EXPECT_EQ(false, isPass);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(SSDTest, ReadTC_ReturnData01)
{
    int lba_addr = 50;
    bool isPass;
    isPass = readcmd.run(lba_addr);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(SSDTest, ReadTC_ReturnData02)
{
    int lba_addr = 30;
    bool isPass;
    isPass = readcmd.run(lba_addr);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(SSDTest, ArgparseRead) {
    string cmd = buildCommand("R", 3);
    ssd.parseCommand(cmd);
    EXPECT_EQ(2, ssd.getArgCount());
    EXPECT_EQ("R", ssd.getOp());
    EXPECT_EQ(3, ssd.getAddr());
}

TEST_F(SSDTest, ArgparseWrite)
{
    string cmd = buildCommand("W", 3, VALID_HEX_DATA);
    ssd.parseCommand(cmd);
    EXPECT_EQ(3, ssd.getArgCount());
    EXPECT_EQ("W", ssd.getOp());
    EXPECT_EQ(3, ssd.getAddr());
    EXPECT_EQ("0x1298CDEF", ssd.getValue());
}

TEST_F(SSDTest, ArgparseInvalidOp)
{
    string cmd = buildCommand("S", 3);
    ssd.parseCommand(cmd);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(SSDTest, ArgparseInvalidAddr)
{
    string cmd = buildCommand("R", 300);
    ssd.parseCommand(cmd);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(SSDTest, ArgparseInvalidValue)
{
    string cmd = buildCommand("W", 3, INVALID_HEX_DATA);
    ssd.parseCommand(cmd);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(SSDTest, WritePass) {
    bool isPass = writecmd.run(VALID_TEST_ADDRESS, VALID_HEX_DATA);
    EXPECT_TRUE(isPass);
}

TEST_F(SSDTest, WriteFailWithOutOfAddressRange) {
    bool isPass = writecmd.run(INVALID_TEST_ADDRESS, VALID_HEX_DATA);
    EXPECT_FALSE(isPass);
}

#ifdef NDEBUG
int main(int argc, char *argv[])
{
    SSD ssd;
    string inputLine;

    // skip ssd.exe myself
    for (int i = 1; i < argc; ++i) {
        if (i > 1) inputLine += " ";
        inputLine += argv[i];
    }

    if (!ssd.parseCommand(inputLine))
        return -1;
    ssd.exec();

    return 0;
}
#else
int main()
{
    ::testing::InitGoogleMock();
    return RUN_ALL_TESTS();
}
#endif