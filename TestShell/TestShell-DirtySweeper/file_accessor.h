#pragma once
#include <string>
#include <iostream>
#include "windows.h"
#include <fstream>
#include <sstream>
using namespace std;

class FileAccessor {
public:
	static FileAccessor& GetInstance();
	string readOutputFile();
	void executeSsdCommandLine(string commandLine);
private:
	FileAccessor() = default;
};