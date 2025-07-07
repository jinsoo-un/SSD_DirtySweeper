#include <iostream>
#include "ssd.cpp"
#include "gmock/gmock.h"

using std::string;

TEST(TS, ARGPARSEREAD) {
    SSD ssd;
    string cmd = "R 3";
    ssd.commandParser(cmd);
    EXPECT_EQ(2, ssd.argCount);
    EXPECT_EQ("R", ssd.op);
    EXPECT_EQ(3, ssd.addr);
}

TEST(TS, ARGPARSEWRITE)
{
    SSD ssd;
    string cmd = "W 3 0x1298CDEF";
    ssd.commandParser(cmd);
    EXPECT_EQ(3, ssd.argCount);
    EXPECT_EQ("W", ssd.op);
    EXPECT_EQ(3, ssd.addr);
    EXPECT_EQ("0x1298CDEF", ssd.value);
}

TEST(TS, ARGPARSEINVALID)
{
    SSD ssd;
    string cmd = "S 3";
    EXPECT_THROW(ssd.commandParser(cmd), std::exception);

}

TEST(TS, ReadTC_InitialValue)
{
    SSD ssd;
    int lba_addr = 0;
    int actual_data;
    actual_data = ssd.readData(lba_addr);
    EXPECT_EQ(0x00000000, actual_data);
}

TEST(TS, ReadTC_OutofRange)
{
    SSD ssd;
    int lba_addr = 100;
    int actual_data;
    actual_data = ssd.readData(lba_addr);
    EXPECT_EQ(-1, actual_data);
}

TEST(TS, ReadTC_ReturnData01)
{
    SSD ssd;
    int lba_addr = 50;
    int actual_data;
    actual_data = ssd.readData(lba_addr);
    EXPECT_EQ(0, actual_data);
}

TEST(TS, ReadTC_ReturnData02)
{
    SSD ssd;
    int lba_addr = 30;
    int actual_data;
    actual_data = ssd.readData(lba_addr);
    EXPECT_EQ(0, actual_data);

}

class SSDTest : public ::testing::Test {
public:
    SSD ssd;
};

TEST_F(SSDTest, WritePass) {
    // Act
    bool isPass = ssd.writeData(0, "0xAAAABBBB");
    EXPECT_TRUE(isPass);
}

TEST_F(SSDTest, WriteFailWithOutOfAddressRange) {
    // Act
    bool isPass = ssd.writeData(100, "0xAAAABBBB");

    // Assert
    EXPECT_FALSE(isPass);
}


int main()
{
    ::testing::InitGoogleMock();
    return RUN_ALL_TESTS();
}