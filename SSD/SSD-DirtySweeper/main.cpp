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
    EXPECT_TRUE(ssd.checkOutputFile("ERROR"));
}

TEST_F(SSDTest, ArgparseInvalidAddr)
{
    SSD ssd;
    string cmd = "R 300";
    ssd.commandParser(cmd);
    EXPECT_TRUE(ssd.checkOutputFile("ERROR"));
}

TEST_F(SSDTest, ArgparseInvalidValue)

{
    SSD ssd;
    string cmd = "W 3 0xABCDEFGH";
    ssd.commandParser(cmd);
    EXPECT_TRUE(ssd.checkOutputFile("ERROR"));
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