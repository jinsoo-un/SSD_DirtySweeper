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
