#include <string>
#include <vector>
#include "file.h"

using std::string;
using std::vector;

class SSDCommand {
public:
	virtual bool run(int addr, string val, int size) = 0;

protected:
	vector<string> ssdData;
	FileControl& file = FileControl::get_instance();
};

class ReadCommand : public SSDCommand {
public:
	bool run(int addr, string val = "0x00000000", int size = 0) override {
		return read(addr, val);
	}
private:
	bool read(int address, string value) {
		if (!file.readData(ssdData)) { file.updateOutput("ERROR");  return false; }

		file.updateOutput(ssdData[address]);
		return true;
	}
};

class WriteCommand : public SSDCommand {
public:
	bool run(int addr, string val, int size = 0) override {
		return writeData(addr, val);
	}
private:
	bool writeData(int address, string hexData) {
		if (!file.readData(ssdData)) { 
			file.updateOutput("ERROR");
			return false; 
		}

		ssdData[address] = hexData;
		if (!file.writeData(ssdData)) { file.updateOutput("ERROR");  return false; };

		file.updateOutput("");

		return true;
	}
};

class EraseCommand : public SSDCommand {
public:
	bool run(int addr, string val, int size) override {
		return erase(addr, val, size);
	}
private:
	bool erase(int address, string val, int size) {
		if (!file.readData(ssdData)) {
			file.updateOutput("ERROR");
			return false;
		}

		for (int i = 0; i < size; i++)
			ssdData[address + i] = "0x00000000";

		if (!file.writeData(ssdData)) {
			file.updateOutput("ERROR");
			return false;
		}

		file.updateOutput("");

		return true;
	}
};