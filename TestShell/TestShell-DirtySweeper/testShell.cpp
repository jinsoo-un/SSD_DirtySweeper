#include <string>
#include <stdexcept>

class SSD {
public:
	virtual void read(int address) = 0;
};

class TestShell {
public:
	TestShell(SSD* ssd) : ssd{ ssd } {}

	void read(int address) {
		if (address < 0 || address > 99) throw std::exception();
		ssd->read(address);
	}
private:
	SSD* ssd;
};