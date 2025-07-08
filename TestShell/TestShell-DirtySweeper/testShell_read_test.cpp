#include "gmock/gmock.h"
#include "testShell.cpp"
using namespace testing;

TEST(ReadTest, BasicRead) {
	SSDMock ssdMock;
	MockTestShell testShell{ &ssdMock };

	EXPECT_CALL(ssdMock, read(_))
		.Times(1);

	EXPECT_CALL(testShell, readOutputFile())
		.WillRepeatedly(Return("0xAAAABBBB"));

	testShell.read(11);
}

TEST(ReadTest, InvalidAddress) {
	SSDMock ssdMock;
	TestShell testShell{ &ssdMock };

	EXPECT_THROW(testShell.read(100), std::exception);
}

TEST(ReadTest, ReadSuccessTest) {
	NiceMock<SSDMock> ssdMock;
	MockTestShell testShell{ &ssdMock };

	EXPECT_CALL(ssdMock, read(_))
		.Times(1);

	EXPECT_CALL(testShell, readOutputFile())
		.WillRepeatedly(Return("0xAAAABBBB"));

	testShell.read(11);
	EXPECT_EQ(testShell.readOutputFile(), "0xAAAABBBB");
}

TEST(ReadTest, ReadFailTest) {
	NiceMock<SSDMock> ssdMock;
	MockTestShell testShell{ &ssdMock };

	EXPECT_CALL(ssdMock, read(_))
		.Times(1);

	EXPECT_CALL(testShell, readOutputFile())
		.WillRepeatedly(Return("ERROR"));

	testShell.read(11);
	EXPECT_EQ(testShell.readOutputFile(), "ERROR");
}

TEST(FullReadTest, TestIfReadCalled100Times) {
	NiceMock<SSDMock> ssdMock;
	MockTestShell testShell{ &ssdMock };

	const int LBA_COUNT = 100;

	EXPECT_CALL(ssdMock, read(_))
		.Times(LBA_COUNT);

	EXPECT_CALL(testShell, readOutputFile())
		.WillRepeatedly(Return("0xAAAABBBB"));

	testShell.fullRead();
}

TEST(FullReadTest, FulLReadFail) {
	NiceMock<SSDMock> ssdMock;
	MockTestShell testShell{ &ssdMock };

	EXPECT_CALL(ssdMock, read(_))
		.Times(4);

	EXPECT_CALL(testShell, readOutputFile())
		.WillOnce(Return("0xAAAABBBB"))
		.WillOnce(Return("0xAAAABBBB"))
		.WillOnce(Return("0xAAAABBBB"))
		.WillRepeatedly(Return("ERROR"));

	testShell.fullRead();
}