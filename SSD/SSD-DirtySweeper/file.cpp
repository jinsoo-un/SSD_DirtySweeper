#include "file.h"
#include <stdexcept>

void FileControl::updateOutput(const string& msg) {
	ofstream fout(FileNames::OUTPUT_FILE);
	if (!fout.is_open())
		std::cout << __func__ << " " << "open fail!" << std::endl;
	fout << msg;
	fout.close();
}

bool FileControl::readData(vector<string>& data) {
	ifstream file(FileNames::DATA_FILE);
	if (!file.is_open()) {
		std::cout << __func__ << " " << "open fail!" << std::endl;
		return false;
	}

	data.clear();
	data.resize(MAX_ADDRESS, "0x00000000"); // 기본값 0으로 초기화

	string line;
	while (getline(file, line)) {
		istringstream iss(line);
		int fileAddress;
		string hexData;
		if (iss >> fileAddress >> hexData) {
			string value = hexData;
			data[fileAddress] = value;
		}
	}
	file.close();
	return true;
}

bool FileControl::writeData(const vector<string>& data)
{
	ofstream file(FileNames::DATA_FILE);
	if (!file.is_open()) {
		std::cout << "Error opening file for writing." << std::endl;
		return false;
	}
	for (int i = 0; i < data.size(); ++i) {
		file << i << "\t" << data[i] << std::endl;
	}
	file.close();

	return true;
}

