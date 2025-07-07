#include "gmock/gmock.h"
#include "testShell.cpp"
using namespace testing;

class SSDMock : public SSD {
public:
	MOCK_METHOD(void, read, (int address), (override));
};

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

	EXPECT_CALL(ssdMock, read(_))
		.Times(0);

	testShell.read(100);
}