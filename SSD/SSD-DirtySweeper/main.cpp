#include "gmock/gmock.h"
#include "ssd.cpp"

TEST(TS, ARGPARSEREAD) {
    SSD ssd;
    string cmd = "R 3";
    ssd.commandParser(cmd);
    EXPECT_EQ(2, ssd.argc);
    EXPECT_EQ("R", ssd.op);
    EXPECT_EQ(3, ssd.addr);
}

TEST(TS, TC1)
{

}

int main()
{
    ::testing::InitGoogleMock();
    return RUN_ALL_TESTS();
}