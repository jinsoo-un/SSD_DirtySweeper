#include "gmock/gmock.h"
#include "testShell.cpp"

using namespace testing;
using std::string;

TEST(HelpTest, HelpCallTest) {
	SSDMock ssd;
	NiceMock<MockTestShell> mockShell{&ssd};

	EXPECT_CALL(mockShell, help())
		.Times(1);

	mockShell.help();
}

TEST(HelpTest, HelpCallTest2) {
	SSDMock ssd;
	TestShell ts{&ssd};
	EXPECT_NO_THROW(ts.help());
}
