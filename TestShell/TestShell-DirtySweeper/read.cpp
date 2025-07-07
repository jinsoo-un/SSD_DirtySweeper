#include "gmock/gmock.h"
#include "testShell.cpp"
using namespace testing;

TEST(ReadTest, TC1) {
	SSDMock ssdMock;
	TestShell testShell{ &ssdMock };

	EXPECT_CALL(ssdMock, read(_))
		.Times(1);

	testShell.read(11);
}

TEST(ReadTest, InvalidAddress) {
	SSDMock ssdMock;
	TestShell testShell{ &ssdMock };

	EXPECT_THROW(testShell.read(100), std::exception);
}

TEST(ReadTest, ReadSuccessTest) {
	NiceMock<SSDMock> ssdMock;
	TestShell testShell{ &ssdMock };

	EXPECT_CALL(ssdMock, read(_))
		.Times(1);

	EXPECT_CALL(ssdMock, readOutputFile())
		.WillRepeatedly(Return("0xAAAABBBB"));

	testShell.read(11);
	EXPECT_EQ(testShell.readOutputFile(), "0xAAAABBBB");
}