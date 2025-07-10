#include "command.h"

string ReadCommand::execute() {
    Logger::GetInstance().print("testShell.read()", "read command called");
    
    if (lba < 0 || lba > 99) {
        return TestShellOutputManager::GetInstance().getErrorReadResult();
    }

    ssd->read(lba);

    string result = FileAccessor::GetInstance()->readOutputFile();
    if (result == "ERROR")  return TestShellOutputManager::GetInstance().getErrorReadResult();
    return TestShellOutputManager::GetInstance().getSuccessReadResult(result, lba);
}

string FullReadCommand::execute() {
    string result = "";
    Logger::GetInstance().print("testShell.fullRead()", "full read command called");

    for (int lba = LBA_START_ADDRESS; lba <= LBA_END_ADDRESS; lba++) {
        ssd->read(lba);
        string outputResult = FileAccessor::GetInstance()->readOutputFile();
        if (outputResult == "ERROR") {
            result += TestShellOutputManager::GetInstance().getErrorReadResult();
            return result;
        }
        result += TestShellOutputManager::GetInstance().getSuccessReadResult(outputResult, lba);
        result += "\n";
    }
    return result;
}

string WriteCommand::execute() {
    Logger::GetInstance().print("testShell.write()", "write command called");
    ssd->write(lba, data);

    if (isCmdExecuteError(FileAccessor::GetInstance()->readOutputFile())) {
        return TestShellOutputManager::GetInstance().getErrorWriteResult();
    }
    return TestShellOutputManager::GetInstance().getSuccessWriteResult();
}

string FullWriteCommand::execute() {
    Logger::GetInstance().print("testShell.fullWrite()", "full write command called");
    for (int lba = LBA_START_ADDRESS; lba <= LBA_END_ADDRESS; lba++) {
        ssd->write(lba, data);

        if (isCmdExecuteError(FileAccessor::GetInstance()->readOutputFile())) {
            return TestShellOutputManager::GetInstance().getErrorFullWriteResult();
        }
    }
    return TestShellOutputManager::GetInstance().getSuccessFullWriteResult();
}

string HelpCommand::execute() {
    Logger::GetInstance().print("testShell.help()", "help command called");

    string output = "Developed by: Team Members - Sooeon Jin, Euncho Bae, Kwangwon Min, Hyeongseok Choi, Yunbae Kim, Seongkyoon Lee\n";
    output += "read (LBA)         : Read data from (LBA).\n";
    output += "write (LBA) (DATA) : Write (DATA) to (LBA).\n";
    output += "fullread           : Read data from all LBAs.\n";
    output += "fullwrite (DATA)   : Write (DATA) to all LBAs\n";
    output += "testscript         : Execute the predefined test script. See documentation for details.\n";
    output += "help               : Show usage instructions for all available commands.\n";
    output += "exit               : Exit the program.\n";
    output += "Note               : INVALID COMMAND will be shown if the input is unrecognized.";
    return output;
}

string ExitCommand::execute() {
    return "Set Exit Comannd...\n";
}

string FullWriteAndReadCompareCommand::execute() {
    Logger::GetInstance().print("testShell.fullWriteAndReadCompare()", "full write and read compare command called");

    for (int lba = LBA_START_ADDRESS; lba <= LBA_END_ADDRESS; ++lba) {
        string writeData = getWriteDataInFullWriteAndReadCompareScript(lba);

        ssd->write(lba, writeData);
        ssd->read(lba);
        string readData = FileAccessor::GetInstance()->readOutputFile();

        if (readData != writeData) {
            return TestShellOutputManager::GetInstance().getWriteReadMismatch(lba, writeData, readData)
                + "\n"
                + TestShellOutputManager::GetInstance().getScriptFailResult();
        }
    }
    return TestShellOutputManager::GetInstance().getScriptPassResult();
}

string FullWriteAndReadCompareCommand::getWriteDataInFullWriteAndReadCompareScript(int lba) {
    string evenData = "0xAAAABBBB";
    string oddData = "0xCCCCDDDD";
    return (lba / 5 % 2 == 0) ? evenData : oddData;
}

string PartialLBAWriteCommand::execute() {
    Logger::GetInstance().print("testShell.partialLBAWrite()", "partial LBA write command called");

    const string testValue = "0x12345678";
    const int repeatCnt = 30;
    for (int count = 1; count <= repeatCnt; count++) {
        ssd->write(4, testValue);
        ssd->write(0, testValue);
        ssd->write(3, testValue);
        ssd->write(1, testValue);
        ssd->write(2, testValue);

        vector<string> result;
        ssd->read(4);
        result.push_back(FileAccessor::GetInstance()->readOutputFile());
        ssd->read(0);
        result.push_back(FileAccessor::GetInstance()->readOutputFile());
        ssd->read(3);
        result.push_back(FileAccessor::GetInstance()->readOutputFile());
        ssd->read(1);
        result.push_back(FileAccessor::GetInstance()->readOutputFile());
        ssd->read(2);
        result.push_back(FileAccessor::GetInstance()->readOutputFile());

        auto firstData = result[0];
        result.erase(result.begin());
        for (auto nextData : result) {
            if (firstData != nextData) {
                return TestShellOutputManager::GetInstance().getScriptFailResult();
            }
        }
    }
    return TestShellOutputManager::GetInstance().getScriptPassResult();
}

string WriteReadAgingCommand::execute() {
    Logger::GetInstance().print("testShell.writeReadAging()", "write read aging command called");

    for (int i = 0; i < WRITE_READ_ITERATION; i++) {
        string randomString = getRandomHexString();
        string firstLBAResult = getWriteReadResult(0, randomString);
        string endLBAResult = getWriteReadResult(99, randomString);

        if (firstLBAResult != endLBAResult) {
            return TestShellOutputManager::GetInstance().getScriptFailResult();
        }
    }
    return TestShellOutputManager::GetInstance().getScriptPassResult();
}

string WriteReadAgingCommand::getWriteReadResult(int lba, string input) {
    ssd->write(lba, input);
    ssd->read(lba);
    string result = FileAccessor::GetInstance()->readOutputFile();
    return result;
}

string Command::getRandomHexString() {
    static const char* hexDigits = "0123456789ABCDEF";

    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned int>(time(nullptr)));
        seeded = true;
    }

    unsigned int value = (static_cast<unsigned int>(rand()) << 16) | rand();

    string result = "0x";
    for (int i = 7; i >= 0; --i) {
        int digit = (value >> (i * 4)) & 0xF;
        result += hexDigits[digit];
    }

    return result;
}

bool EraseWithSizeCommand::isValidEraseWithSizeArgument(unsigned int lba, unsigned int size) {
    if (lba > LBA_END_ADDRESS) {
        return false;
    }

    if (size < 1 || size > 100) {
        return false;
    }
    if (lba + size > LBA_END_ADDRESS + 1) {
        return false;
    }
    return true;
}
string EraseWithSizeCommand::execute() {
    Logger::GetInstance().print("testShell.eraseWithSize()", "erase with size command called");

    if (!isValidEraseWithSizeArgument(lba, size)) {
        return TestShellOutputManager::GetInstance().getEraseErrorResult();
    }

    string result = erase(lba, size);
    if (isCmdExecuteError(result)) {
        return TestShellOutputManager::GetInstance().getEraseErrorResult();
    }

    return TestShellOutputManager::GetInstance().getErasePassResult();
}

string Command::erase(unsigned int lba, unsigned int size) {
    const int maxEraseSize = 10;
    int currentLba = lba;
    for (int remainedSize = size; remainedSize > 0;) {
        int chunkSize = min(maxEraseSize, remainedSize);
        ssd->erase(currentLba, chunkSize);
        remainedSize -= chunkSize;
        currentLba += chunkSize;
        if (FileAccessor::GetInstance()->readOutputFile() == "ERROR") {
            return "ERROR";
        }
    }
    return "Done";
}

string EraseWithRangeCommand::execute() {
    Logger::GetInstance().print("testShell.eraseWithRange()", "erase with range command called");

    if (!isValidLbaRange(startLba, endLba)) {
        return TestShellOutputManager::GetInstance().getEraseRangeErrorResult();
    }

    const unsigned int size = endLba - startLba + 1;
    string result = erase(startLba, size);
    if (isCmdExecuteError(result)) {
        return TestShellOutputManager::GetInstance().getEraseRangeErrorResult();
    }

    return TestShellOutputManager::GetInstance().getEraseRangePassResult();
}

bool EraseWithRangeCommand::isValidLbaRange(unsigned int startLba, unsigned int endLba)
{
    if (startLba < LBA_START_ADDRESS || startLba > LBA_END_ADDRESS) {
        return false;
    }

    if (endLba < LBA_START_ADDRESS || endLba > LBA_END_ADDRESS) {
        return false;
    }

    if (startLba > endLba) {
        return false;
    }
    return true;
}

string EraseAndWriteAgingCommand::execute() {
    Logger::GetInstance().print("testShell.eraseAndWriteAging()", "erase and write aging command called");

    const int eraseUnitSize = 2;
    const int maxAgingCnt = 30;
    ssd->erase(0, eraseUnitSize);

    if (isCmdExecuteError(FileAccessor::GetInstance()->readOutputFile())) {
        return TestShellOutputManager::GetInstance().getScriptFailResult();
    }

    for (int loopCnt = 0; loopCnt < maxAgingCnt; loopCnt++) {
        for (int lba = 2; lba < LBA_END_ADDRESS; lba += eraseUnitSize) {
            vector<string> result;
            ssd->write(lba, getRandomHexString());
            result.push_back(FileAccessor::GetInstance()->readOutputFile());
            ssd->write(lba, getRandomHexString());
            result.push_back(FileAccessor::GetInstance()->readOutputFile());
            ssd->erase(lba, eraseUnitSize);
            result.push_back(FileAccessor::GetInstance()->readOutputFile());

            for (auto data : result) {
                if (isCmdExecuteError(data)) {
                    return TestShellOutputManager::GetInstance().getScriptFailResult();
                }
            }
        }
    }
    return TestShellOutputManager::GetInstance().getScriptPassResult();
}

string FlushCommand::execute() {
    Logger::GetInstance().print("testShell.flushSsdBuffer()", "flush command called");
    ssd->flushSsdBuffer();
    string result = FileAccessor::GetInstance()->readOutputFile();
    if (result == "ERROR") return TestShellOutputManager::GetInstance().getErrorFlushResult();
    return TestShellOutputManager::GetInstance().getSuccessFlushResult();
}

unique_ptr<Command> CommandFactory::getCommand(SSD* ssd, string cmd, const vector<string>& args) {
    if (cmd == "read") {
        int lba = stoi(args[0]);
        return std::make_unique<ReadCommand>(ssd, lba);
    }
    if (cmd == "fullread") {
        return std::make_unique<FullReadCommand>(ssd);
    }

    if (cmd == "write") {
        int lba = stoi(args[0]);
        string data = args[1];
        return std::make_unique<WriteCommand>(ssd, lba, data);
    }

    if (cmd == "fullwrite") {
        string data = args[0];
        return make_unique<FullWriteCommand>(ssd, data);
    }

    if (cmd == "help") {
        return make_unique<HelpCommand>(ssd);
    }

    if (cmd == "exit") {
        return make_unique<ExitCommand>(ssd);
    }

    if (cmd == "1_" || cmd == "1_FullWriteAndReadCompare") {
        return make_unique<FullWriteAndReadCompareCommand>(ssd);
    }

    if (cmd == "2_" || cmd == "2_PartialLBAWrite") {
        return make_unique<PartialLBAWriteCommand>(ssd);
    }
    if (cmd == "3_" || cmd == "3_WriteReadAging") {
        return make_unique<WriteReadAgingCommand>(ssd);
    }

    if (cmd == "erase") {
        int lba = stoi(args[0]);
        int size = stoi(args[1]);
        return make_unique<EraseWithSizeCommand>(ssd, lba, size);
    }

    if (cmd == "erase_range") {
        int startLba = stoi(args[0]);
        int endLba = stoi(args[1]);
        return make_unique<EraseWithRangeCommand>(ssd, startLba, endLba);
    }

    if (cmd == "4_" || cmd == "4_EraseAndWriteAging") {
        return make_unique<EraseAndWriteAgingCommand>(ssd);
    }

    if (cmd == "flush") {
        return make_unique<FlushCommand>(ssd);
    }

    return nullptr;
}

bool ReadCommand::isArgumentSizeValid(int argsSize) {
    if (argsSize != 1) return false;
    return true;
}

bool FullReadCommand::isArgumentSizeValid(int argsSize) {
    if (argsSize != 0) return false;
    return true;
}

bool WriteCommand::isArgumentSizeValid(int argsSize) {
    if (argsSize != 2) return false;
    return true;
}

bool FullWriteCommand::isArgumentSizeValid(int argsSize) {
    if (argsSize != 1) return false;
    return true;
}

bool HelpCommand::isArgumentSizeValid(int argsSize) {
    return true;
}
bool ExitCommand::isArgumentSizeValid(int argsSize) {
    return true;
}
bool FullWriteAndReadCompareCommand::isArgumentSizeValid(int argsSize) {
    return true;
}
bool PartialLBAWriteCommand::isArgumentSizeValid(int argsSize) {
    return true;
}
bool WriteReadAgingCommand::isArgumentSizeValid(int argsSize) {
    return true;
}

bool EraseWithSizeCommand::isArgumentSizeValid(int argsSize) {
    if (argsSize != 2) return false;
    return true;
}

bool EraseWithRangeCommand::isArgumentSizeValid(int argsSize) {
    if (argsSize != 2) return false;
    return true;
}

bool EraseAndWriteAgingCommand::isArgumentSizeValid(int argsSize) {
    return true;
}

bool FlushCommand::isArgumentSizeValid(int argsSize) {
    return true;
}