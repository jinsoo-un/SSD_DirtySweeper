
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <stdexcept>
using namespace std;

using std::string;

namespace FileNames {
	const std::string DATA_FILE = "ssd_nand.txt";
	const std::string OUTPUT_FILE = "ssd_output.txt";
}

class SSD {
public:
	void erase() {
		ofstream file(FileNames::DATA_FILE);
		if (!file.is_open()) {
			cout << "Error opening file for writing." << endl;
			return;
		}
		for (int i = 0; i < ssdData.size(); ++i) {
			file << i << "\t" << "0x00000000" << endl;
		}

		file.close();

		ssdData.clear();
		ssdData.resize(MAX_ADDRESS, "0x00000000");
	}

	void commandParser(string command) {
		std::istringstream iss(command);
		string arg;
		int cnt = 0;

		if (!isValidCommand(command)) {
			updateOutputFile("ERROR");
			return;
		}

		/* scan the command line input */
		while (iss >> arg) {
			cnt++;
			if (cnt == 1)
				op = arg;
			if (cnt == 2)
				addr = std::stoi(arg);
			if (cnt == 3)
				value = arg;
		}
		argCount = cnt;
	}

	bool readData(int address) {
        ifstream ssd_file("ssd_nand.txt");
		ofstream output_file("ssd_output.txt");
        
		if (!ssd_file.is_open() || !output_file.is_open()) {
			throw std::exception();
		}

		if (true == isAddressOutOfRange(address))
		{
			updateOutputFile("ERROR");
			return false;
		}

		string line;
        for (int addr = 0; addr < MAX_ADDRESS; addr++) {
			getline(ssd_file, line);		

            if (addr == address)
            {
				output_file << line;
            }                
        }

		ssd_file.close();
		output_file.close();
        return true;
      
	}

	bool writeData(int address, string hexData) {
		if (isAddressOutOfRange(address)) { updateOutputFile("ERROR");  return false; }
		if (!readFromFile()) { updateOutputFile("ERROR");  return false; }

		ssdData[address] = hexData;
		if (!writeFileFromData()) { updateOutputFile("ERROR");  return false; };

		updateOutputFile("");

		return true;
	}

	bool exec() {
		if (op == "R")
			return readData(addr);
		if (op == "W")
			return writeData(addr, value);
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

private:
	bool isAddressOutOfRange(int address)
	{
		return address < MIN_ADDRESS || address >= MAX_ADDRESS;
	}

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

	void updateOutputFile(string msg) {
		ofstream fout(FileNames::OUTPUT_FILE);
		fout << msg;
		fout.close();
	}

	bool isValidCommand(string command) {
		std::istringstream iss(command);
		string arg;
		int cnt = 0;

		while (iss >> arg) {
			cnt++;
			if (cnt == 1) {
				if (!isValidOp(arg)) return false;
			}
			else if (cnt == 2) {
				if (isAddressOutOfRange(stoi(arg))) return false;
			}
			else if (cnt == 3) {
				if (!isHexWithPrefix(arg)) return false;
			}
			else
				return false;
		}

		return true;
	}

	bool isValidOp(string arg) {
		if (arg != "R" && arg != "W")
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

	int argCount;
	string op;
	int addr;
	string value;

	static const int MIN_ADDRESS = 0;
	static const int MAX_ADDRESS = 100;

	vector<string> ssdData; // Simulated SSD data storage
	

};