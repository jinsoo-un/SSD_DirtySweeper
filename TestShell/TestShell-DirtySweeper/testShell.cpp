#include "testShell.h"

using namespace std;

string TestShell::executeCommand(const string& cmd, const vector<string>& args) {

    const string INVALID_COMMAND = "INVALID COMMAND";

    if (cmd == "read") {
        int lba = stoi(args[0]);
        return read(lba);
    }

    if (cmd == "fullread") {
        return fullRead();
    }

    if (cmd == "write") {
        int lba = stoi(args[0]);
        string data = args[1];
        return write(lba, data);
    }

    if (cmd == "fullwrite") {
        string data = args[0];
        return fullWrite(data);
    }

    if (cmd == "help") {
        return help();
    }

    if (cmd == "exit") {
        return exit();
    }

    if (cmd == "1_" || cmd == "1_FullWriteAndReadCompare") {
        return fullWriteAndReadCompare();
    }

    if (cmd == "2_" || cmd == "2_PartialLBAWrite") {
        return partialLBAWrite();
    }
    if (cmd == "3_" || cmd == "3_WriteReadAging") {
        return writeReadAging();
    }

    if (cmd == "erase") {
        if (args.size() != 2) {
            return INVALID_COMMAND;
        }
        int lba = stoi(args[0]);
        int size = stoi(args[1]);
        return eraseWithSize(lba, size);
    }

    if (cmd == "erase_range") {
        if (args.size() != 2) {
            return INVALID_COMMAND;
        }
        int startLba = stoi(args[0]);
        int endLba = stoi(args[1]);
        return eraseWithRange(startLba, endLba);
    }
    if (cmd == "4_" || cmd == "4_EraseAndWriteAging") {
        return eraseAndWriteAging();
    }

    if (cmd == "flush") {
        return flushSsdBuffer();
    }

    return INVALID_COMMAND;
}

void TestShell::processInput(const string& input) {
    auto tokens = tokenize(input);
    if (tokens.empty()) {
        return;
    }

    const string& cmd = tokens[0];
    vector<string> args(tokens.begin() + 1, tokens.end());

    if (isValidCommand(cmd) && isArgumentSizeValid(cmd, args.size())) {
        auto result = executeCommand(cmd, args);
        cout << result << endl;
        return;
    }

    cout << "INVALID COMMAND\n";
}

string TestShell::help() {
    logger.print("testShell.help()", "help command called");

    string output = "Developed by: Team Members - Sooeon Jin, Euncho Bae, Kwangwon Min, Hyeongseok Choi, Yunbae Kim, Seongkyoon Lee\n";
    output += "read (LBA)                    : Read data from (LBA).\n";
    output += "write (LBA) (DATA)            : Write (DATA) to (LBA).\n";
    output += "fullread                      : Read data from all LBAs.\n";
    output += "fullwrite (DATA)              : Write (DATA) to all LBAs\n";
    output += "erase (LBA) (SIZE)            : Erase  data from LBA to LBA + SIZE - 1\n";
    output += "erase_range (ST_LBA) (EN LBA) : Erase  data from ST_LBA to EN_LBA\n";
    output += "flush                         : Flush entire buffer\n";
    output += "1_FullWriteAndReadCompare     : Write and Read for All LBAs. Also you can execute with '1_' .See documetation for details.\n";
    output += "2_PartialLBAWrite             : Aging test for partial LBA write and read. Also you can execute with '2_'. See documetation for details\n";
    output += "3_WriteReadAging              : Write and Read aging test.Also you can execute with '3_'.See documetation for details\n";
    output += "4_EraseAndWriteAging          : Erase and Write aging test. Also you can execute with '4_'. See documetation for details\n";
    output += "flush                         : Flush entire buffer\n";
    output += "help                          : Show usage instructions for all available commands.\n";
    output += "exit                          : Exit the program.\n";
    output += "Note                          : INVALID COMMAND will be shown if the input is unrecognized.";
    return output;
}

string TestShell::read(int lba) {
    logger.print("testShell.read()", "read command called");

    if (lba < 0 || lba > 99) {
        return testShellStringManager.getErrorReadResult();
    }
    ssd->read(lba);

    string result = fileAccessor->readOutputFile();
    if (result == "ERROR")  return testShellStringManager.getErrorReadResult();
    return testShellStringManager.getSuccessReadResult(result, lba);
}

string TestShell::fullRead() {
    string result = "";
    logger.print("testShell.fullRead()", "full read command called");

    for (int lba = LBA_START_ADDRESS; lba <= LBA_END_ADDRESS; lba++) {
        ssd->read(lba);
        string outputResult = fileAccessor->readOutputFile();
        if (outputResult == "ERROR") {
            result += testShellStringManager.getErrorReadResult();
            return result;
        }
        result += testShellStringManager.getSuccessReadResult(outputResult, lba);
        result += "\n";
    }
    return result;
}

string TestShell::write(int lba, string data)
{
    logger.print("testShell.write()", "write command called");
    ssd->write(lba, data);

    if (isCmdExecuteError(fileAccessor->readOutputFile())) {
        return testShellStringManager.getErrorWriteResult();
    }
    return testShellStringManager.getSuccessWriteResult();
}

string TestShell::fullWrite(string data) {
    logger.print("testShell.fullWrite()", "full write command called");
    for (int lba = LBA_START_ADDRESS; lba <= LBA_END_ADDRESS; lba++) {
        ssd->write(lba, data);

        if (isCmdExecuteError(fileAccessor->readOutputFile())) {
            return testShellStringManager.getErrorFullWriteResult();
        }
    }
    return testShellStringManager.getSuccessFullWriteResult();
}

string TestShell::fullWriteAndReadCompare() {
    logger.print("testShell.fullWriteAndReadCompare()", "full write and read compare command called");

    for (int lba = LBA_START_ADDRESS; lba <= LBA_END_ADDRESS; ++lba) {
        string writeData = getWriteDataInFullWriteAndReadCompareScript(lba);

        ssd->write(lba, writeData);
        ssd->read(lba);
        string readData = fileAccessor->readOutputFile();

        if (readData != writeData) {
            return testShellStringManager.getWriteReadMismatch(lba, writeData, readData)
                + "\n"
                + testShellStringManager.getScriptFailResult();
        }
    }
    return testShellStringManager.getScriptPassResult();
}

string TestShell::exit(void) {
    isExitCmd = true;
    return "Set Exit Comannd...\n";
}
bool TestShell::isExit() const {
    return isExitCmd;
}

string TestShell::writeReadAging() {
    logger.print("testShell.writeReadAging()", "write read aging command called");

    for (int i = 0; i < WRITE_READ_ITERATION; i++) {
        string randomString = getRandomHexString();
        string firstLBAResult = getWriteReadResult(0, randomString);
        string endLBAResult = getWriteReadResult(99, randomString);

        if (firstLBAResult != endLBAResult) {
            return testShellStringManager.getScriptFailResult();
        }
    }
    return testShellStringManager.getScriptPassResult();
}

string TestShell::getRandomHexString() {
    return generateRandomHexString();
}

string TestShell::partialLBAWrite() {
    logger.print("testShell.partialLBAWrite()", "partial LBA write command called");

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
        result.push_back(fileAccessor->readOutputFile());
        ssd->read(0);
        result.push_back(fileAccessor->readOutputFile());
        ssd->read(3);
        result.push_back(fileAccessor->readOutputFile());
        ssd->read(1);
        result.push_back(fileAccessor->readOutputFile());
        ssd->read(2);
        result.push_back(fileAccessor->readOutputFile());

        auto firstData = result[0];
        result.erase(result.begin());
        for (auto nextData : result) {
            if (firstData != nextData) {
                return testShellStringManager.getScriptFailResult();
            }
        }
    }
    return testShellStringManager.getScriptPassResult();
}

string TestShell::eraseWithSize(unsigned int lba, unsigned int size) {
    logger.print("testShell.eraseWithSize()", "erase with size command called");

    if (!isValidEraseWithSizeArgument(lba, size)) {
        return testShellStringManager.getEraseErrorResult();
    }

    string result = erase(lba, size);
    if (isCmdExecuteError(result)) {
        return testShellStringManager.getEraseErrorResult();
    }

    return testShellStringManager.getErasePassResult();
}

string TestShell::eraseWithRange(unsigned int startLba, unsigned int endLba) {
    logger.print("testShell.eraseWithRange()", "erase with range command called");

    if (!isValidLbaRange(startLba, endLba)) {
        return testShellStringManager.getEraseRangeErrorResult();
    }

    const unsigned int size = endLba - startLba + 1;
    string result = erase(startLba, size);
    if (isCmdExecuteError(result)) {
        return testShellStringManager.getEraseRangeErrorResult();
    }

    return testShellStringManager.getEraseRangePassResult();
}

string TestShell::eraseAndWriteAging(void) {
    logger.print("testShell.eraseAndWriteAging()", "erase and write aging command called");

    const int eraseUnitSize = 2;
    const int maxAgingCnt = 30;
    ssd->erase(0, eraseUnitSize);

    if (isCmdExecuteError(fileAccessor->readOutputFile())) {
        return testShellStringManager.getScriptFailResult();
    }

    for (int loopCnt = 0; loopCnt < maxAgingCnt; loopCnt++) {
        for (int lba = 2; lba < LBA_END_ADDRESS; lba += eraseUnitSize) {
            vector<string> result;
            ssd->write(lba, getRandomHexString());
            result.push_back(fileAccessor->readOutputFile());
            ssd->write(lba, getRandomHexString());
            result.push_back(fileAccessor->readOutputFile());
            ssd->erase(lba, eraseUnitSize);
            result.push_back(fileAccessor->readOutputFile());

            for (auto data : result) {
                if (isCmdExecuteError(data)) {
                    return testShellStringManager.getScriptFailResult();
                }
            }
        }
    }
    return testShellStringManager.getScriptPassResult();
}

string TestShell::flushSsdBuffer(void) {
    logger.print("testShell.flushSsdBuffer()", "flush command called");    
    ssd->flushSsdBuffer();
    string result = fileAccessor->readOutputFile();
    if (result == "ERROR") return testShellStringManager.getErrorFlushResult();
    return testShellStringManager.getSuccessFlushResult();
}

bool TestShell::isArgumentSizeValid(const string& cmd, int argsSize) {
    if (cmd == "read") {
        if (argsSize != 1) return false;
    }
    else if (cmd == "fullread") {
        if (argsSize != 0) return false;
    }
    else if (cmd == "write") {
        if (argsSize != 2) return false;
    }
    else if (cmd == "fullwrite") {
        if (argsSize != 1) return false;
    }
    return true;
}

string TestShell::readOutputFile() {
    // shell.exe의 절대 경로 구하기
    char modulePath[MAX_PATH];
    GetModuleFileNameA(NULL, modulePath, MAX_PATH);

    string shellFullPath(modulePath);
    size_t lastSlash = shellFullPath.find_last_of("\\/");
    string shellDir = (lastSlash != string::npos) ? shellFullPath.substr(0, lastSlash) : ".";

    // ssd_output.txt의 상대 경로 → 절대 경로 변환
    string relativePath = shellDir + "\\..\\..\\..\\SSD\\x64\\Release\\ssd_output.txt";

    char absPath[MAX_PATH];
    _fullpath(absPath, relativePath.c_str(), MAX_PATH);

    ifstream file(absPath);
    if (!file.is_open()) {
        cerr << "Failed to open output file: " << absPath << endl;
        throw exception();
    }

    ostringstream content;
    string line;
    while (getline(file, line)) {
        content << line;
    }

    return content.str();
}

string TestShell::getWriteReadResult(int lba, string input) {
    ssd->write(lba, input);
    ssd->read(lba);
    string result = fileAccessor->readOutputFile();
    return result;
}

vector<string> TestShell::tokenize(const string& input) {
    vector<string> tokens;
    istringstream iss(input);
    string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

bool TestShell::isValidCommand(const string& cmd) const {
    static const unordered_set<string> valid = {
        "read", "write", "exit", "help", "fullread", "fullwrite",
        "testscript", "1_", "1_FullWriteAndReadCompare",
        "2_","2_PartialLBAWrite",
        "3_", "3_WriteReadAging",
        "erase","erase_range",
        "4_", "4_EraseAndWriteAging",
        "flush"
    };
    return valid.count(cmd) > 0;
}

string TestShell::erase(unsigned int lba, unsigned int size) {
    const int maxEraseSize = 10;
    int currentLba = lba;
    for (int remainedSize = size; remainedSize > 0;) {
        int chunkSize = min(maxEraseSize, remainedSize);
        ssd->erase(currentLba, chunkSize);
        remainedSize -= chunkSize;
        currentLba += chunkSize;
        if (fileAccessor->readOutputFile() == "ERROR") {
            return "ERROR";
        }
    }
    return "Done";
}

bool TestShell::isValidLbaRange(unsigned int startLba, unsigned int endLba)
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

bool TestShell::isValidEraseWithSizeArgument(unsigned int lba, unsigned int size) {
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

bool TestShell::isCmdExecuteError(const string result) const {
    return result == "ERROR";
}

string TestShell::generateRandomHexString() {
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

string TestShell::getWriteDataInFullWriteAndReadCompareScript(int lba) {
    string evenData = "0xAAAABBBB";
    string oddData = "0xCCCCDDDD";
    return (lba / 5 % 2 == 0) ? evenData : oddData;
}

