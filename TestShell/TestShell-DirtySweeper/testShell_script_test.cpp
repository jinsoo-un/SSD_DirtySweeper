#include "gmock/gmock.h"
#include "testShell.cpp"
using namespace testing;

TEST(ScriptTest, TC1) {
	EXPECT_EQ(1, 1);
}

TEST(WriteReadAging, TC1) {
    testing::NiceMock<SSDMock> ssdMock;
    TestShell sut(&ssdMock);
    
    const string actual = "[Write] Done";
    const string DATA = "0xAAAABBBB";

    EXPECT_CALL(ssdMock, write(0, DATA))
        .Times(200);
    EXPECT_CALL(ssdMock, write(99, DATA))
        .Times(200);
    EXPECT_CALL(ssdMock, read(0))
        .Times(200);
    EXPECT_CALL(ssdMock, read(99))
        .Times(200);

    sut.writeReadAging();
}