#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using std::string;
using std::ifstream;
using std::ofstream;
using std::vector;
using std::istringstream;

namespace FileNames {
	const std::string DATA_FILE = "ssd_nand.txt";
	const std::string OUTPUT_FILE = "ssd_output.txt";
}

const int MAX_ADDRESS = 100;

// Singleton FileControl Class
// ex : FileControl& file = FileControl::get_instance();
class FileControl {
private:
	FileControl() {}
	FileControl(const FileControl& c) = delete;
	FileControl& operator=(const FileControl&) = delete;
public:
	static FileControl& get_instance() {
		static FileControl instance;
		return instance;
	}
	void updateOutput(const string& msg);
	bool readData(vector<string>& data);
	bool writeData(const vector<string>& data);
};