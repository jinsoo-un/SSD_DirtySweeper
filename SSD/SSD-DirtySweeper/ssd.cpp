
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <stdexcept>
using namespace std;

using std::string;

const int VALID_DATA_LENGTH = 10;
const int MIN_ADDRESS = 0;
const int MAX_ADDRESS = 100;

namespace FileNames {
    const std::string DATA_FILE = "ssd_nand.txt";
    const std::string OUTPUT_FILE = "ssd_output.txt";
}

class SSDCommand {
public:
	virtual bool run(int addr, string val, int size) = 0;

protected:
	bool readFromFile() {
		ifstream file(FileNames::DATA_FILE);
		if (!file.is_open()) {
			return false;
		}

		ssdData.clear();
		ssdData.resize(MAX_ADDRESS, "0x00000000"); // 기본값 0으로 초기화

		string line;
		while (getline(file, line)) {
			istringstream iss(line);
			int fileAddress;
			string hexData;
			if (iss >> fileAddress >> hexData) {
				string value = hexData;
				ssdData[fileAddress] = value;
			}
		}
		file.close();
	}

	bool writeFileFromData(void)
	{
		ofstream file(FileNames::DATA_FILE);
		if (!file.is_open()) {
			cout << "Error opening file for writing." << endl;
			return false;
		}
		for (int i = 0; i < ssdData.size(); ++i) {
			file << i << "\t" << ssdData[i] << endl;
		}
		file.close();

		return true;
	}

	bool isAddressOutOfRange(int address) {
		return address < MIN_ADDRESS || address >= MAX_ADDRESS;
	}

	void updateOutputFile(string msg) {
		ofstream fout(FileNames::OUTPUT_FILE);
		fout << msg;
		fout.close();
	}

	vector<string> ssdData;
};

class ReadCommand : public SSDCommand {
public:
	bool run(int addr, string val = "0x00000000", int size = 0) override {
		return readData(addr, val);
	}
private:
	bool readData(int address, string value) {
		if (isAddressOutOfRange(address)) { updateOutputFile("ERROR");  return false; }
		if (!readFromFile()) { updateOutputFile("ERROR");  return false; }

		updateOutputFile(ssdData[address]);
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
		if (isAddressOutOfRange(address)) { updateOutputFile("ERROR");  return false; }
		if (!isValidWriteData(hexData)) { updateOutputFile("ERROR");  return false; }
		if (!readFromFile()) { updateOutputFile("ERROR");  return false; }

		ssdData[address] = hexData;
		if (!writeFileFromData()) { updateOutputFile("ERROR");  return false; };

		updateOutputFile("");

		return true;
	}

	bool isValidWriteData(const std::string& str) {

		if (str.substr(0, 2) != "0x") return false;

		int length;
		for (length = 2; length < str.size(); length++) {
			char ch = static_cast<unsigned char>(str[length]);

			if (!(isNumber(ch) || isHexCharacter(ch))) { return false; }

		}

		if (length != VALID_DATA_LENGTH) { return false; }

		return true;
	}

	bool isHexCharacter(char ch)
	{
		return ((ch >= 'A') && (ch <= 'F'));
	}

	bool isNumber(char ch)
	{
		return ((ch >= '0') && (ch <= '9'));
	}
};

class EraseCommand : public SSDCommand {
public:
	bool run(int addr, string val, int size) override {
		return erase(addr, val, size);
	}
private:
	bool erase(int address, string val, int size) {
		if (address + size >= MAX_ADDRESS) {
			updateOutputFile("ERROR");
			return false;
		}
		if (size < 1 || size > 10) {
			updateOutputFile("ERROR");
			return false;
		}

		if (!readFromFile()) { 
			updateOutputFile("ERROR");  
			return false; 
		}

		for (int i = 0; i < size; i++)
			ssdData[address + i] = "0x00000000";
		
		if (!writeFileFromData()) {
			updateOutputFile("ERROR");  
			return false; 
		}

		updateOutputFile("");

		return true;
	}
};

// SSD Interface Class
class SSD {
public:
	virtual bool parseCommand(string command) = 0;
	virtual void exec() = 0;
	virtual int getArgCount() = 0;
	virtual string getOp() = 0;
	virtual int getAddr() = 0;
	virtual string getValue() = 0;
    virtual int getAccessCount() = 0;
};

class RealSSD : public SSD {
public:
	bool parseCommand(string command) {
        if (!isValidCommand(command)) {
	        updateOutputFile("ERROR");
	        return false;
        }
        storeParams(command);
        return true;
	}

	void exec() {
		ReadCommand readCmd;
		WriteCommand writeCmd;
		EraseCommand eraseCmd;

		if (op == "R")
			setCommand(&readCmd);
		if (op == "W")
			setCommand(&writeCmd);
		if (op == "E")
			setCommand(&eraseCmd);

		if (command == nullptr)
			return;

		command->run(addr, value, size);
	}

	int getArgCount() {
		return argCount;
	}

	string getOp() {
		return op;
	}

	int getAddr() {
		return addr;
	}

	string getValue() {
		return value;
	}

    int getAccessCount() {
        return accessCount;
    }

private:
	void storeParams(string command)
	{
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
			if (cnt == 3) {
				if (op == "E") size = std::stoi(arg);
				else value = arg;
			}
        }
        argCount = cnt;
	}

	bool isAddressOutOfRange(int address) {
		return address < MIN_ADDRESS || address >= MAX_ADDRESS;
	}

	void updateOutputFile(string msg) {
        ofstream fout(FileNames::OUTPUT_FILE);
        fout << msg;
        fout.close();
	}

	bool isValidCommand(string command) {
        std::istringstream iss(command);
        string arg;
        int cnt = 0;
		bool isErase = false;

        while (iss >> arg) {
	        cnt++;
	        if (cnt == 1) {
		        if (!isValidOp(arg)) return false;
				if (arg == "E") isErase = true;
	        }
	        else if (cnt == 2) {
		        if (isAddressOutOfRange(stoi(arg))) return false;
	        }
	        else if (cnt == 3) {
				if (isErase) continue;
		        if (!isHexWithPrefix(arg)) return false;
	        }
	        else
		        return false;
        }

        return true;
	}

	bool isValidOp(string arg) {
        if (arg != "R" && arg != "W" && arg != "E")
	        return false;
        return true;
	}

	bool isHexWithPrefix(const std::string& str) {
        if (str.size() < 3 || str.substr(0, 2) != "0x")
	        return false;
        for (size_t i = 2; i < str.size(); ++i) {
	        if (!std::isxdigit(static_cast<unsigned char>(str[i])))
		        return false;
        }
        return true;
	}

	void setCommand(SSDCommand* cmd) {
		command = cmd;
	}

	int argCount;
	string op;
	int addr;
	string value;
	int size;
    int accessCount;

	SSDCommand* command = nullptr;
};


// SSD Proxy Class
class BufferedSSD : public SSD {
public:
	BufferedSSD() : ssd{ new RealSSD() } {}
	bool parseCommand(string command) {
		return ssd->parseCommand(command);
	}
	void exec() {
		string operation = ssd->getOp();
		if (operation == "R") read();
		if (operation == "W") write();
		if (operation == "E") erase();	
	}
	int getArgCount() {
		return ssd->getArgCount();
	}
	string getOp() {
		return ssd->getOp();
	}
	int getAddr() {
		return ssd->getAddr();
	}
	string getValue() {
		return ssd->getValue();
	}

    int getAccessCount() {
        return ssd->getAccessCount();
    }
private:
	// Buffered SSD methods
	void read() {
		// Check buffer first
		// if buffer is empty, read from RealSSD
		ssd->exec(); // read from RealSSD
	}

	void write() {
		// check if buffer is full, flush to RealSSD
		// check if command can be merged with buffer
		// if buffer has room, write to buffer
		ssd->exec(); // write to RealSSD
	}

	void erase() {
		// check if buffer is full, flush to RealSSD
		// check if command can be merged with buffer
		// if buffer has room, write to buffer
		ssd->exec(); // Erase RealSSD
	}

	RealSSD* ssd; // RealSSD instance

};

