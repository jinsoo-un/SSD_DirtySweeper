#include <string>

class SSD {
public:
	virtual void read(int address) = 0;
};

class TestShell {
public:
	TestShell(SSD* ssd) : ssd{ ssd } {}

	void read(int address) {
		ssd->read(address);
	}
private:
	SSD* ssd;
};