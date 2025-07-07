#include "gmock/gmock.h"
#include "SSD.cpp"


TEST(SSDTest, WritePass) {
    // Arrange
    SSD ssd;

    // Act
    bool isPass = ssd.writeData(0, 0xAAAABBBB);
    EXPECT_TRUE(isPass);
    
    int readData = ssd.readData(0);
   	EXPECT_EQ(readData, 0xAAAABBBB);
}

TEST(SSDTest, WriteFailWithOutOfAddressRange) {
    // Arrange
    SSD ssd;

    // Act
    bool isPass = ssd.writeData(100, 0xAAAABBBB);

    // Assert
    EXPECT_FALSE(isPass);
}


int main()
{
    ::testing::InitGoogleMock();
    return RUN_ALL_TESTS();
}