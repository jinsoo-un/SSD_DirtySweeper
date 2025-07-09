#include "gmock/gmock.h"
#include "testShell.cpp"
using namespace testing;

class ReadTestFixture : public Test {
public:
	NiceMock<SSDMock> ssdMock;
	MockTestShell testShell{ &ssdMock };

	const std::string RANDOM_VALUE = "0xAAAABBBB";
	const std::string ERROR_RESULT = "ERROR";
	const int RANDOM_LBA = 11;
	const int LBA_COUNT = 100;
};

TEST_F(ReadTestFixture, BasicRead) {
	EXPECT_CALL(ssdMock, read(_))
		.Times(1);

	EXPECT_CALL(testShell, readOutputFile())
		.WillRepeatedly(Return(RANDOM_VALUE));

	testShell.read(RANDOM_LBA);
}

TEST_F(ReadTestFixture, InvalidAddress) {
	EXPECT_CALL(ssdMock, read(_))
		.Times(0);

	EXPECT_CALL(testShell, readOutputFile())
		.Times(0);

	testing::internal::CaptureStdout();
	testShell.read(100);
	std::string output = testing::internal::GetCapturedStdout();
	cout << output;
	EXPECT_THAT(output, ::testing::HasSubstr("ERROR"));
}

TEST_F(ReadTestFixture, ReadSuccessTest) {
	EXPECT_CALL(ssdMock, read(_))
		.Times(1);

	EXPECT_CALL(testShell, readOutputFile())
		.WillRepeatedly(Return(RANDOM_VALUE));

	testShell.read(RANDOM_LBA);
	EXPECT_EQ(testShell.readOutputFile(), RANDOM_VALUE);
}

TEST_F(ReadTestFixture, ReadFailTest) {
	EXPECT_CALL(ssdMock, read(_))
		.Times(1);

	EXPECT_CALL(testShell, readOutputFile())
		.WillRepeatedly(Return(ERROR_RESULT));

	testShell.read(11);
	EXPECT_EQ(testShell.readOutputFile(), ERROR_RESULT);
}

TEST_F(ReadTestFixture, TestIfReadCalled100Times) {
	EXPECT_CALL(ssdMock, read(_))
		.Times(LBA_COUNT);

	EXPECT_CALL(testShell, readOutputFile())
		.WillRepeatedly(Return(RANDOM_VALUE));

	testShell.fullRead();
}

TEST_F(ReadTestFixture, FulLReadFail) {
	EXPECT_CALL(ssdMock, read(_))
		.Times(4);

	EXPECT_CALL(testShell, readOutputFile())
		.WillOnce(Return(RANDOM_VALUE))
		.WillOnce(Return(RANDOM_VALUE))
		.WillOnce(Return(RANDOM_VALUE))
		.WillRepeatedly(Return(ERROR_RESULT));

	testShell.fullRead();
}
