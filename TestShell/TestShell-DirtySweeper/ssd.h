#pragma once
#include "gmock/gmock.h"
#include "logger.h"
#include "file_accessor.h"

using namespace std;

class SSD {
public:
    virtual void read(int lba) = 0;
    virtual void write(int lba, string data) = 0;
    virtual void erase(unsigned int lba, unsigned int size) = 0;
    virtual void flushSsdBuffer(void) = 0;
};

class SsdHelpler : public SSD {
public:
    void read(int lba) override;
    void write(int lba, string data) override;
    void erase(unsigned int lba, unsigned size) override;
    void flushSsdBuffer(void) override;

private:
    Logger& logger{ Logger::GetInstance() };
    string buildCommandLine(string cmd, int lba = 0, string data = "");
    void executeCommandLine(string commandLine);
};

class SSDMock : public SSD {
public:
    MOCK_METHOD(void, read, (int lba), (override));
    MOCK_METHOD(void, write, (int, string), (override));
    MOCK_METHOD(void, erase, (unsigned int, unsigned int), (override));
    MOCK_METHOD(void, flushSsdBuffer, (), (override));
};