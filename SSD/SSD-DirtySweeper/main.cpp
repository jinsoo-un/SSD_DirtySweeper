#include <iostream>
#include "ssd.cpp"
#include "gmock/gmock.h"

using std::string;

class SSDTest : public ::testing::Test {
public:
    SSD ssd;

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

TEST_F(SSDTest, ArgparseRead) {
    string cmd = "R 3";
    ssd.commandParser(cmd);
    EXPECT_EQ(2, ssd.getArgCount());
    EXPECT_EQ("R", ssd.getOp());
    EXPECT_EQ(3, ssd.getAddr());
}

TEST_F(SSDTest, ArgparseWrite)
{
    string cmd = "W 3 0x1298CDEF";
    ssd.commandParser(cmd);
    EXPECT_EQ(3, ssd.getArgCount());
    EXPECT_EQ("W", ssd.getOp());
    EXPECT_EQ(3, ssd.getAddr());
    EXPECT_EQ("0x1298CDEF", ssd.getValue());
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
    bool isPass = ssd.writeData(0, "0xAAAABBBB");
    EXPECT_TRUE(isPass);
}

TEST_F(SSDTest, WriteFailWithOutOfAddressRange) {
    bool isPass = ssd.writeData(100, "0xAAAABBBB");

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