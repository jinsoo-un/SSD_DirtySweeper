#include <iostream>
#include "ssd.cpp"
#include "gmock/gmock.h"

using std::string;

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

int main()
{
    ::testing::InitGoogleMock();
    return RUN_ALL_TESTS();
}