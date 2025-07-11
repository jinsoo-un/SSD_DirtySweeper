#pragma once
#include "logger.h"
#include "testShell_output_manager.h"
#include "file_accessor.h"
#include "ssd.h"
#include <memory>

class ICommand {
public:
    ICommand(SSDInterface* ssd) : ssd{ ssd } {
#ifdef NDEBUG
        fileAccessor = FileAccessor::GetInstance();
#else
        fileAccessor = MockFileAccessor::GetInstance();
#endif
    }
    virtual string execute() = 0;
    
protected:
    SSDInterface* ssd;
    IFileAccessor* fileAccessor;
    const int LBA_START_ADDRESS = 0;
    const int LBA_END_ADDRESS = 99;

    
    bool isCmdExecuteError(const string result) const {
        return result == "ERROR";
    }
    string getRandomHexString();

    string erase(unsigned int lba, unsigned int size);
};

class ReadCommand : public ICommand {
public:
    ReadCommand(SSDInterface* ssd, int lba) : ICommand(ssd), lba{ lba } {}
    string execute() override;

private:
    int lba;
};

class FullReadCommand : public ICommand {
public:
    FullReadCommand(SSDInterface* ssd) : ICommand(ssd) {}
    string execute() override;
};

class WriteCommand : public ICommand {
public:
    WriteCommand(SSDInterface* ssd, int lba, string data) : ICommand(ssd), lba{ lba }, data{data} {}
    string execute() override;
private:
    int lba;
    string data;
};

class FullWriteCommand : public ICommand {
public:
    FullWriteCommand(SSDInterface* ssd, string data) : ICommand(ssd), data{ data } {}
    string execute() override;
private:
    string data;
};

class HelpCommand : public ICommand {
public:
    HelpCommand(SSDInterface* ssd) : ICommand(ssd) {}
    string execute() override;
};

class ExitCommand : public ICommand {
public:
    ExitCommand(SSDInterface* ssd) : ICommand(ssd) {}
    string execute() override;
    static bool getIsExit();
private:
    static bool isExit;
};

class FullWriteAndReadCompareCommand : public ICommand {
public:
    FullWriteAndReadCompareCommand(SSDInterface* ssd) : ICommand(ssd) {}
    string execute() override;
private:
    string getWriteDataInFullWriteAndReadCompareScript(int lba);
};

class PartialLBAWriteCommand : public ICommand {
public:
    PartialLBAWriteCommand(SSDInterface* ssd) : ICommand(ssd) {}
    string execute() override;
};

class WriteReadAgingCommand : public ICommand {
public:
    WriteReadAgingCommand(SSDInterface* ssd) : ICommand(ssd) {}
    string execute() override;
    static const int WRITE_READ_ITERATION = 200;
private:
    string getWriteReadResult(int lba, string input);
};

class EraseWithSizeCommand : public ICommand {
public:
    EraseWithSizeCommand(SSDInterface* ssd, int lba, int size) : ICommand(ssd), lba{ lba }, size{size} {}
    string execute() override;
private:
    int lba;
    int size;
    bool isValidEraseWithSizeArgument(unsigned int lba, unsigned int size);
};

class EraseWithRangeCommand : public ICommand {
public:
    EraseWithRangeCommand(SSDInterface* ssd, int startLba, int endLba) : ICommand(ssd), startLba{ startLba }, endLba{ endLba } {}
    string execute() override;
private:
    int startLba;
    int endLba;
    bool isValidLbaRange(unsigned int startLba, unsigned int endLba);
};

class EraseAndWriteAgingCommand : public ICommand {
public:
    EraseAndWriteAgingCommand(SSDInterface* ssd) : ICommand(ssd) {}
    string execute() override;
};

class FlushCommand : public ICommand {
public:
    FlushCommand(SSDInterface* ssd) : ICommand(ssd) {}
    string execute() override;
};

class InvalidCommand : public ICommand {
public:
    InvalidCommand(SSDInterface* ssd) : ICommand(ssd) {}
    string execute() override;
};


class CommandFactory {
public:
    unique_ptr<ICommand> getCommand(SSDInterface* ssd, string cmd, const vector<string>& args);
    bool isArgumentSizeValid(string cmd, int argsSize);
    static bool isExit();
};
