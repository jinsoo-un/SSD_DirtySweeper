#pragma once
#include "logger.h"
#include "testShell_output_manager.h"
#include "file_accessor.h"
#include "ssd.h"
#include <memory>

class Command {
public:
    Command(SSD* ssd) : ssd{ ssd } {
#ifdef NDEBUG
        fileAccessor = FileAccessor::GetInstance();
#else
        fileAccessor = MockFileAccessor::GetInstance();
#endif
    }
    virtual string execute() = 0;
    virtual bool isArgumentSizeValid(int argsSize) = 0;
    
protected:
    SSD* ssd;
    IFileAccessor* fileAccessor;
    const int LBA_START_ADDRESS = 0;
    const int LBA_END_ADDRESS = 99;

    
    bool isCmdExecuteError(const string result) const {
        return result == "ERROR";
    }
    string getRandomHexString();

    string erase(unsigned int lba, unsigned int size);
};

class ReadCommand : public Command {
public:
    ReadCommand(SSD* ssd, int lba) : Command(ssd), lba{ lba } {}
    string execute() override;
    bool isArgumentSizeValid(int argsSize) override;

private:
    int lba;
};

class FullReadCommand : public Command {
public:
    FullReadCommand(SSD* ssd) : Command(ssd) {}
    string execute() override;
    bool isArgumentSizeValid(int argsSize) override;
};

class WriteCommand : public Command {
public:
    WriteCommand(SSD* ssd, int lba, string data) : Command(ssd), lba{ lba }, data{data} {}
    string execute() override;
    bool isArgumentSizeValid(int argsSize) override;
private:
    int lba;
    string data;
};

class FullWriteCommand : public Command {
public:
    FullWriteCommand(SSD* ssd, string data) : Command(ssd), data{ data } {}
    string execute() override;
    bool isArgumentSizeValid(int argsSize) override;
private:
    string data;
};

class HelpCommand : public Command {
public:
    HelpCommand(SSD* ssd) : Command(ssd) {}
    string execute() override;
    bool isArgumentSizeValid(int argsSize) override;
};

class ExitCommand : public Command {
public:
    ExitCommand(SSD* ssd) : Command(ssd) {}
    string execute() override;
    bool isArgumentSizeValid(int argsSize) override;
};

class FullWriteAndReadCompareCommand : public Command {
public:
    FullWriteAndReadCompareCommand(SSD* ssd) : Command(ssd) {}
    string execute() override;
    bool isArgumentSizeValid(int argsSize) override;
private:
    string getWriteDataInFullWriteAndReadCompareScript(int lba);
};

class PartialLBAWriteCommand : public Command {
public:
    PartialLBAWriteCommand(SSD* ssd) : Command(ssd) {}
    string execute() override;
    bool isArgumentSizeValid(int argsSize) override;
};

class WriteReadAgingCommand : public Command {
public:
    WriteReadAgingCommand(SSD* ssd) : Command(ssd) {}
    string execute() override;
    bool isArgumentSizeValid(int argsSize) override;
private:
    static const int WRITE_READ_ITERATION = 200;
    string getWriteReadResult(int lba, string input);
};

class EraseWithSizeCommand : public Command {
public:
    EraseWithSizeCommand(SSD* ssd, int lba, int size) : Command(ssd), lba{ lba }, size{size} {}
    string execute() override;
    bool isArgumentSizeValid(int argsSize) override;
private:
    int lba;
    int size;
    bool isValidEraseWithSizeArgument(unsigned int lba, unsigned int size);
};

class EraseWithRangeCommand : public Command {
public:
    EraseWithRangeCommand(SSD* ssd, int startLba, int endLba) : Command(ssd), startLba{ startLba }, endLba{ endLba } {}
    string execute() override;
    bool isArgumentSizeValid(int argsSize) override;
private:
    int startLba;
    int endLba;
    bool isValidLbaRange(unsigned int startLba, unsigned int endLba);
};


class EraseAndWriteAgingCommand : public Command {
public:
    EraseAndWriteAgingCommand(SSD* ssd) : Command(ssd) {}
    string execute() override;
    bool isArgumentSizeValid(int argsSize) override;
};

class FlushCommand : public Command {
public:
    FlushCommand(SSD* ssd) : Command(ssd) {}
    string execute() override;
    bool isArgumentSizeValid(int argsSize) override;
};


class CommandFactory {
public:
    unique_ptr<Command> getCommand(SSD* ssd, string cmd, const vector<string>& args);
};
