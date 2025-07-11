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
    TestShell(SSDInterface* ssd) : ssd{ ssd } {
    }

    string executeCommand(const string& cmd, const vector<string>& args);
    void processInput(const string& input);
    bool isExit() const;
    
private:
    vector<string> tokenize(const string& input);

    SSDInterface* ssd;
    bool isExitCmd{ false };
};

class MockTestShell : public TestShell {
public:
    MockTestShell(SSDInterface* ssd) : TestShell(ssd) {}    
};
