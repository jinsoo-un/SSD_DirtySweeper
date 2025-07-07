#include "gmock/gmock.h"
#include "testShell.cpp"

using namespace testing;
using std::string;

class MockTestShell : public TestShell {
public:
	MOCK_METHOD(void, help, (), ());
};

TEST(HelpTest, HelpCallTest) {
	NiceMock<MockTestShell> mockShell;

	EXPECT_CALL(mockShell, help())
		.Times(1);

	mockShell.help();
}

TEST(HelpTest, HelpCallTest2) {
	TestShell ts;
	EXPECT_NO_THROW(ts.help());
}
