#include "gmock/gmock.h"
#include "testShell.cpp"

int main() {
	::testing::InitGoogleMock();
	return RUN_ALL_TESTS();
}
