#pragma once
using namespace std;

class TestShellOutputManager {
public:
	static TestShellOutputManager& GetInstance();
	
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
	string getSuccessFlushResult();
	string getErrorFlushResult();

private:
	TestShellOutputManager() = default;
	TestShellOutputManager(const TestShellOutputManager& c) = delete;
	TestShellOutputManager& operator=(const TestShellOutputManager&) = delete;
};
