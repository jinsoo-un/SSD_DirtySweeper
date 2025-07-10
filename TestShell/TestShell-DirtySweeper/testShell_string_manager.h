#pragma once
using namespace std;

class TestShellStringManager {
public:
	string getErrorReadResult();
	string getSuccessReadResult(string result, int lba);
	string getSuccessWriteResult();
	string getErrorWriteResult();
	string getSuccessFullWriteResult();
	string getErrorFullWriteResult();
	string getScriptFailResult();
	string getScriptPassResult();
	string getWriteReadMismatch(int lba, string writeData, string readData);
	string getEraseErrorResult();
	string getErasePassResult();
	string getEraseRangeErrorResult();
	string getEraseRangePassResult();
	void printSuccessFlushResult();
	void printErrorFlushResult();

	string generateRandomHexString();
	string getWriteDataInFullWriteAndReadCompareScript(int lba);
	string getNewLine();
};
