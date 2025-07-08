
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <stdexcept>
using namespace std;

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
				checkOp(arg);
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
			createErrorOutputFile();
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
		createErrorOutputFile();
		if (isAddressOutOfRange(address)) return false; 
		if (!readFromFile()) return false;
		
		ssdData[address] = hexData;

		// Write data to ssd_nand.txt file
		if (!writeFileFromData()) return false;

		// Write ssd_output.txt file as null
		createEmptyOutputFile();
		
		return true;
	}

	void checkOp(string arg) {
		if (arg != "R" && arg != "W")
			throw std::exception();
		op = arg;
	}

	bool exec() {
		if (op == "R")
			return readData(addr);
		if (op == "W")
			return writeData(addr, value);
	}

	int argCount;
	string op;
	int addr;
	string value;

private:
	bool isAddressOutOfRange(int address)
	{
		return address < MIN_ADDRESS || address >= MAX_ADDRESS;
	}

	bool readFromFile() {
		ifstream file("ssd_nand.txt");
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
		ofstream file("ssd_nand.txt");
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

	void createEmptyOutputFile() {
		ofstream file("ssd_output.txt", ios::trunc);
		file.close();
	}

	void createErrorOutputFile() {
		string msg{ "ERROR" };
		ofstream fout2("ssd_output.txt");
		fout2 << msg;
		fout2.close();
	}

	static const int MIN_ADDRESS = 0;
	static const int MAX_ADDRESS = 100;
	vector<string> ssdData; // Simulated SSD data storage
	

};