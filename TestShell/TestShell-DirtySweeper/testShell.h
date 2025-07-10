#pragma once
#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <windows.h>
#include <shellapi.h>
#include <direct.h>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <io.h>
#include "gmock/gmock.h"
#include "testShell_string_manager.h"
#include "logger.h"
#include "ssd.h"

using namespace std;

class TestShell {
public:
    TestShell(SSD* ssd) : ssd{ ssd } {}

    void executeCommand(const string& cmd, const vector<string>& args);
    void processInput(const string& input);
    void help();
    void read(int lba);
    void fullRead();
    void write(int lba, string data);
    void fullWrite(string data);
    void fullWriteAndReadCompare();
    void exit(void);
    bool isExit() const;
    void writeReadAging();
    virtual string getRandomHexString();
    void partialLBAWrite();
    void eraseWithSize(unsigned int lba, unsigned int size);
    void eraseWithRange(unsigned int startLba, unsigned int endLba);
    void eraseAndWriteAging(void);
    void flushSsdBuffer(void);

    static const int WRITE_READ_ITERATION = 200;
private:
    bool isArgumentSizeValid(const string& cmd, int argsSize);
    virtual string readOutputFile();
    string getWriteReadResult(int lba, string input);
    vector<string> tokenize(const string& input);
    bool isValidCommand(const string& cmd) const;
    string erase(unsigned int lba, unsigned int size);
    bool isValidLbaRange(unsigned int startLba, unsigned int endLba);
    bool isValidEraseWithSizeArgument(unsigned int lba, unsigned int size);
    bool isCmdExecuteError(const string result) const;

    SSD* ssd;
    Logger& logger{ Logger::GetInstance()};
    TestShellStringManager testShellStringManager;

    bool isExitCmd{ false };
    const int LBA_START_ADDRESS = 0;
    const int LBA_END_ADDRESS = 99;
};

class MockTestShell : public TestShell {
public:
    MockTestShell(SSD* ssd) : TestShell(ssd) {}
    MOCK_METHOD(void, help, (), ());
    MOCK_METHOD(string, readOutputFile, (), ());
    MOCK_METHOD(string, getRandomHexString, (), ());
};
