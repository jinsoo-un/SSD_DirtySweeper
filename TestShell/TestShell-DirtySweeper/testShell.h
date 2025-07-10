#pragma once
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <unordered_set>
#include "gmock/gmock.h"
#include "testShell_output_manager.h"
#include "logger.h"
#include "ssd.h"
#include "testShell.h"
#include "file_accessor.h"

using namespace std;
class TestShell {
public:
    TestShell(SSD* ssd) : ssd{ ssd } {
    }

    string executeCommand(const string& cmd, const vector<string>& args);
    void processInput(const string& input);
    bool isExit() const;
    
private:
    bool isArgumentSizeValid(const string& cmd, int argsSize);
    vector<string> tokenize(const string& input);
    bool isValidCommand(const string& cmd) const;

    SSD* ssd;
    bool isExitCmd{ false };
};

class MockTestShell : public TestShell {
public:
    MockTestShell(SSD* ssd) : TestShell(ssd) {}
    MOCK_METHOD(void, help, (), ());
    MOCK_METHOD(string, readOutputFile, (), ());
    MOCK_METHOD(string, getRandomHexString, (), ());
};
