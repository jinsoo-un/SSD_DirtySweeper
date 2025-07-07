#include "gmock/gmock.h"
#include "SSD.cpp"

class SSDTest : public ::testing::Test {
public:
    // Arrange
    SSD ssd;
};


TEST_F(SSDTest, WritePass) {
    // Act
    bool isPass = ssd.writeData(0, "0xAAAABBBB");
    EXPECT_TRUE(isPass);
    
    //string readData = ssd.readData(0);
   	//EXPECT_EQ(readData, "0xAAAABBBB");
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