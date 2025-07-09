#pragma once
class TestShellStringManager {
public:
    void printErrorReadResult();
	void printSuccessReadResult(std::string result, int lba);
	void printSuccessWriteResult();
	void printErrorWriteResult();
	void printSuccessFullWriteResult();
	void printErrorFullWriteResult();
	void printScriptFailResult();
	void printScriptPassResult();
	void printWriteReadMismatch(int lba, std::string writeData, std::string readData);
	std::string generateRandomHexString();
	std::string getWriteDataInFullWriteAndReadCompareScript(int lba);
};

namespace SSDPathManager {
	static const std::string filePath = "..\\..\\SSD\\x64\\Release\\ssd.exe";
	static const std::string workingDir = "..\\..\\SSD\\x64\\Release";
	static const std::string outputPath = "..\\..\\SSD\\x64\\\Release\\ssd_output.txt";
};
