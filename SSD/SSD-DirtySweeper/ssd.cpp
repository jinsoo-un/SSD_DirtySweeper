#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>

using std::string;

class SSD {
public:
	void commandParser(string command) {
		std::istringstream iss(command);
		string arg;
		int cnt = 0;

		/* scan the command line input */
		while (iss >> arg) {
			cnt++;
			if (cnt == 1)
				op = arg;
			if (cnt == 2)
				addr = std::stoi(arg);
			if (cnt == 3)
				value = std::stoul(arg, nullptr, 0);
		}

		if (op != "R" && op != "W")
			throw std::exception();

		argCount = cnt;
	}
	void readData(int address) {

	}
	void writeData(int address, int data) {

	}

	int argCount;
	string op;
	int addr;
	unsigned int value;
};