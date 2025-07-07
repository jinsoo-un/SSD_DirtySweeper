#include<iostream>
#include <sstream>
#include <vector>

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
		}

		argc = cnt;
	}
	void readData(int address) {

	}
	void writeData(int address, int data) {

	}

	int argc;
	string op;
	int addr;
};