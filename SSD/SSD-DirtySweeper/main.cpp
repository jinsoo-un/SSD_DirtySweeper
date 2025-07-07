#include "gmock/gmock.h"
#include "ssd.cpp"

class SSDTest : public ::testing::Test {
public:
    // Arrange
    SSD ssd;
};

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