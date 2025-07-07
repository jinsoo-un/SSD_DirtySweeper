#include "gmock/gmock.h"
#include "ssd.cpp"

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
    EXPECT_EQ(0x1298CDEF, ssd.value);
}

int main()
{
    ::testing::InitGoogleMock();
    return RUN_ALL_TESTS();
}