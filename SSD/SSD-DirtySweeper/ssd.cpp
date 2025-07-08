
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <stdexcept>
using namespace std;

using std::string;

// 파일명 상수들을 위한 namespace
namespace FileNames {
	const std::string DATA_FILE = "ssd_nand.txt";
	const std::string OUTPUT_FILE = "ssd_output.txt";
}


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

	int readData(int address) {

        //generate init ssd_nand file
        if ((address < 0) || (address >= 100)){
            //Error
            string msg{ "ERROR" };
            ofstream fout2(FileNames::OUTPUT_FILE);
            fout2 << msg;
            fout2.close();
            return -1;
        }


        ifstream fin(FileNames::DATA_FILE);
        ofstream fout2(FileNames::OUTPUT_FILE);
            
        char line[20];

        for (int i = 0; i < 100; i++) {
            fin.getline(line, 20);

            if (i == address)
            {
                fout2 << line;
            }                
        }

        fin.close();
        fout2.close();
        return 0;
      
	}

	bool writeData(int address, string hexData) {
		if (isAddressOutOfRange(address)) { updateOutputFile("ERROR");  return false; }
		if (!readFromFile()) { updateOutputFile("ERROR");  return false; }

		ssdData[address] = hexData;
		if (!writeFileFromData()) { updateOutputFile("ERROR");  return false; };

		updateOutputFile("");

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

	static const int MIN_ADDRESS = 0;
	static const int MAX_ADDRESS = 100;

	vector<string> ssdData; // Simulated SSD data storage
	

};