#include <iostream>
#include "ssd.cpp"
#include "gmock/gmock.h"

using std::string;

class SSDTest : public ::testing::Test {
public:
    SSD ssd;
    string VALID_HEX_DATA = "0x1298CDEF";
    string INVALID_HEX_DATA = "0xABCDEFGH";
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
};


TEST_F(SSDTest, ReadTC_InitialValue)
{
    int lba_addr = 1;
    bool isPass;
    isPass = ssd.readData(lba_addr);
    EXPECT_EQ(true, isPass);
}

TEST_F(SSDTest, ReadTC_OutofRange)
{
    int lba_addr = 100;
    bool isPass;
    isPass = ssd.readData(lba_addr);
    EXPECT_EQ(false, isPass);
}

TEST_F(SSDTest, ReadTC_ReturnData01)
{
    int lba_addr = 50;
    bool isPass;
    isPass = ssd.readData(lba_addr);
    EXPECT_EQ(true, isPass);
}

TEST_F(SSDTest, ReadTC_ReturnData02)
{
    int lba_addr = 30;
    bool isPass;
    isPass = ssd.readData(lba_addr);
    EXPECT_EQ(true, isPass);

}

TEST_F(SSDTest, ARGPARSEREAD) {
    string cmd = "R 3";
    ssd.commandParser(cmd);
    EXPECT_EQ(2, ssd.argCount);
    EXPECT_EQ("R", ssd.op);
    EXPECT_EQ(3, ssd.addr);
}

TEST_F(SSDTest, ARGPARSEWRITE)
{
    string cmd = "W 3 0x1298CDEF";
    ssd.commandParser(cmd);
    EXPECT_EQ(3, ssd.argCount);
    EXPECT_EQ("W", ssd.op);
    EXPECT_EQ(3, ssd.addr);
    EXPECT_EQ("0x1298CDEF", ssd.value);
}

TEST_F(SSDTest, ArgparseInvalidOp)
{
    string cmd = "S 3";
    ssd.commandParser(cmd);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(SSDTest, ArgparseInvalidAddr)
{
    SSD ssd;
    string cmd = "R 300";
    ssd.commandParser(cmd);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(SSDTest, ArgparseInvalidValue)

{
    SSD ssd;
    string cmd = "W 3 0xABCDEFGH";
    ssd.commandParser(cmd);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}


TEST_F(SSDTest, WritePass) {
    bool isPass = ssd.writeData(VALID_TEST_ADDRESS, VALID_HEX_DATA);
    EXPECT_TRUE(isPass);
}

TEST_F(SSDTest, WriteFailWithOutOfAddressRange) {
    bool isPass = ssd.writeData(INVALID_TEST_ADDRESS, VALID_HEX_DATA);
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

    ssd.commandParser(inputLine);
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