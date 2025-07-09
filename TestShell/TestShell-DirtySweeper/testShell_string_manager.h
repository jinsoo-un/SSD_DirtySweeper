#pragma once
using namespace std;

class TestShellStringManager {
public:
    void printErrorReadResult();
	void printSuccessReadResult(string result, int lba);
	void printSuccessWriteResult();
	void printErrorWriteResult();
	void printSuccessFullWriteResult();
	void printErrorFullWriteResult();
	void printScriptFailResult();
	void printScriptPassResult();
	void printWriteReadMismatch(int lba, string writeData, string readData);
	string generateRandomHexString();
	string getWriteDataInFullWriteAndReadCompareScript(int lba);
};
