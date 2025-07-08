#include "gmock/gmock.h"
#include "testShell.cpp"

using namespace testing;
using std::string;

TEST(ExitTest, DefaultExitStatus) {
	SSDMock ssd;
	TestShell sut{ &ssd };

	EXPECT_FALSE(sut.isExit());
}

TEST(ExitTest, AfterExitCmd) {
	SSDMock ssd;
	TestShell sut{ &ssd };

	sut.exit();
	EXPECT_TRUE(sut.isExit());
}