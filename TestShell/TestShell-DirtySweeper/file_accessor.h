#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "windows.h"
#include "gmock/gmock.h"

using namespace std;

class IFileAccessor {
public:
    virtual string readOutputFile() = 0;
    virtual void executeSsdCommandLine(string commandLine) = 0;
};

class FileAccessor : public IFileAccessor{
public:
    static IFileAccessor* GetInstance();
    string readOutputFile();
    void executeSsdCommandLine(string commandLine);
private:
    FileAccessor() = default;
};

class MockFileAccessor : public IFileAccessor {
public:
    static IFileAccessor* GetInstance();
    MOCK_METHOD(string, readOutputFile, (), (override));
    MOCK_METHOD(void, executeSsdCommandLine, (string commandLine), (override));
};
