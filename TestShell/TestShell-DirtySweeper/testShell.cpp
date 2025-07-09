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

using namespace std;

class Logger {
public:
    void print(const string& sender, const string& message) {
        rotateIfNeeded();

        ofstream logFile(LOG_FILE_NAME, ios::app);
        if (!logFile.is_open()) return;

        logFile << "[" << currentDateTime() << "] ";
        logFile << padRight(sender, 30) << ": " << message << "\n";
    }

private:
    const string LOG_FILE_NAME = "latest.log";
    const size_t MAX_LOG_FILE_SIZE = 10 * 1024; // 10KB

    string currentDateTime() {
        time_t now = time(nullptr);
        tm tmStruct;
        localtime_s(&tmStruct, &now);
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%y.%m.%d %H:%M", &tmStruct);
        return buffer;
    }

    string formatForFileName(tm tmStruct) {
        ostringstream oss;
        oss << "until_"
            << put_time(&tmStruct, "%y%m%d_%Hh_%Mm_%Ss")
            << ".log";
        return oss.str();
    }

    string padRight(const string& str, size_t width) {
        if (str.length() >= width) return str;
        return str + string(width - str.length(), ' ');
    }

    size_t getFileSize(const string& path) {
        struct _stat st;
        if (_stat(path.c_str(), &st) != 0) return 0;
        return st.st_size;
    }

    void rotateIfNeeded() {
        size_t size = getFileSize(LOG_FILE_NAME);
        if (size < MAX_LOG_FILE_SIZE) return;

        // rename latest.log to until_YYMMDD_HHh_MMm_SSs.log
        time_t now = time(nullptr);
        tm tmStruct;
        localtime_s(&tmStruct, &now);
        string newFileName = formatForFileName(tmStruct);

        rename(LOG_FILE_NAME.c_str(), newFileName.c_str());

        handleUntilLogFiles();
    }

    void handleUntilLogFiles() {
        vector<string> untilLogs;

        _finddata_t fileinfo;
        intptr_t handle = _findfirst("until_*.log", &fileinfo);
        if (handle != -1) {
            do {
                untilLogs.push_back(fileinfo.name);
            } while (_findnext(handle, &fileinfo) == 0);
            _findclose(handle);
        }

        if (untilLogs.size() < 2) return;

        // find oldest file
        string oldest = untilLogs[0];
        time_t oldestTime = getLastWriteTime(oldest);

        for (const auto& f : untilLogs) {
            time_t t = getLastWriteTime(f);
            if (t < oldestTime) {
                oldestTime = t;
                oldest = f;
            }
        }

        // rename .log to .zip
        string zipName = oldest.substr(0, oldest.find_last_of('.')) + ".zip";
        rename(oldest.c_str(), zipName.c_str());
    }

    time_t getLastWriteTime(const string& path) {
        WIN32_FILE_ATTRIBUTE_DATA info;
        if (!GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &info)) return 0;

        FILETIME ft = info.ftLastWriteTime;
        ULARGE_INTEGER ull;
        ull.LowPart = ft.dwLowDateTime;
        ull.HighPart = ft.dwHighDateTime;
        return static_cast<time_t>((ull.QuadPart - 116444736000000000ULL) / 10000000ULL);
    }
};

class SSD {
public:
    virtual void read(int lba) = 0;
    virtual void write(int lba, string data) = 0;
    virtual void erase(unsigned int lba, unsigned int size) = 0;
};

class SsdHelpler : public SSD {
public:
    string buildCommandLine(string cmd, int lba, string data = "") {
        string cmdLine = cmd + " " + to_string(lba);
        if (cmd == "W" || cmd == "E") cmdLine = cmdLine + " " + data;
        return cmdLine;
    }

    void executeCommandLine(string commandLine) {

        char modulePath[MAX_PATH];
        GetModuleFileNameA(NULL, modulePath, MAX_PATH);

        string shellFullPath(modulePath);
        size_t lastSlash = shellFullPath.find_last_of("\\/");
        string shellDir = (lastSlash != string::npos) ? shellFullPath.substr(0, lastSlash) : ".";

        // ssd.exe 경로: shellDir 기준으로 상대 위치
        string ssdRelativePath = shellDir + "\\..\\..\\..\\SSD\\x64\\Release\\ssd.exe";
        string workingDirRelative = shellDir + "\\..\\..\\..\\SSD\\x64\\Release";

        // 절대경로로 변환
        char absSsdPath[MAX_PATH];
        _fullpath(absSsdPath, ssdRelativePath.c_str(), MAX_PATH);

        char absWorkingDir[MAX_PATH];
        _fullpath(absWorkingDir, workingDirRelative.c_str(), MAX_PATH);

        string fullCommand = "\"" + string(absSsdPath) + "\" " + commandLine;

        STARTUPINFOA si = { sizeof(STARTUPINFOA) };
        PROCESS_INFORMATION pi;

        BOOL success = CreateProcessA(
            NULL,
            &fullCommand[0],  // 반드시 non-const!
            NULL, NULL, FALSE,
            0,
            NULL,
            absWorkingDir,
            &si, &pi
        );

        if (!success) {
            cerr << "CreateProcess failed with error: " << GetLastError() << endl;
            return;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    void read(int lba)  override {
        logger.print("SsdHelpler.read()", "Reading LBA: " + to_string(lba));
        string commandLine = buildCommandLine("R", lba);
        executeCommandLine(commandLine);
    }

    void write(int lba, string data) override {
        logger.print("SsdHelpler.write()", "Writing to LBA: " + to_string(lba) + " with data: " + data);
        string commandLine = buildCommandLine("W", lba, data);
        executeCommandLine(commandLine);
    }
    void erase(unsigned int lba, unsigned size) override {
        logger.print("SsdHelpler.erase()", "Erasing LBA: " + to_string(lba) + " with size: " + to_string(size));
        string commandLine = buildCommandLine("E", lba, to_string(size));
        executeCommandLine(commandLine);
    }

private:
    Logger logger;
};

class SSDMock : public SSD {
public:
    MOCK_METHOD(void, read, (int lba), (override));
    MOCK_METHOD(void, write, (int, string), (override));
    MOCK_METHOD(void, erase, (unsigned int, unsigned int), (override));
};

class TestShell {
public:
    TestShell(SSD* ssd) : ssd{ ssd } {}

    void executeCommand(const string& cmd, const vector<string>& args) {
        if (cmd == "read") {
            int lba = stoi(args[0]);
            this->read(lba);
            return;
        }

        if (cmd == "fullread") {
            this->fullRead();
            return;
        }

        if (cmd == "write") {
            int lba = stoi(args[0]);
            string data = args[1];
            this->write(lba, data);
            return;
        }

        if (cmd == "fullwrite") {
            string data = args[0];
            this->fullWrite(data);
            return;
        }

        if (cmd == "help") {
            this->help();
            return;
        }

        if (cmd == "exit") {
            this->exit();
            return;
        }

        if (cmd == "1_" || cmd == "1_FullWriteAndReadCompare") {
            fullWriteAndReadCompare();
            return;
        }

        if (cmd == "2_" || cmd == "2_PartialLBAWrite") {
            this->partialLBAWrite();
            return;
        }
        if (cmd == "3_" || cmd == "3_WriteReadAging") {
            this->writeReadAging();
            return;
        }

        if (cmd == "erase") {
            if (args.size() != 2) {
                cout << "INVALID COMMAND\n";
                return;
            }
            int lba = stoi(args[0]);
            int size = stoi(args[1]);
            this->eraseWithSize(lba, size);
            return;
        }

        if (cmd == "erase_range") {
            if (args.size() != 2) {
                cout << "INVALID COMMAND\n";
                return;
            }
            int startLba = stoi(args[0]);
            int endLba = stoi(args[1]);
            this->eraseWithRange(startLba, endLba);
            return;
        }
        if (cmd == "4_" || cmd == "4_EraseAndWriteAging") {
            this->eraseAndWriteAging();
            return;
        }
        cout << "INVALID COMMAND\n";
    }

    void processInput(const string& input) {
        auto tokens = tokenize(input);
        if (tokens.empty()) {
            return;
        }

        const string& cmd = tokens[0];
        vector<string> args(tokens.begin() + 1, tokens.end());

        if (isValidCommand(cmd) && isArgumentSizeValid(cmd, args.size())) {
            executeCommand(cmd, args);
            return;
        }

        cout << "INVALID COMMAND\n";
    }

    void help() {
        logger.print("testShell.help()", "help command called");

        cout << "Developed by: Team Members - Sooeon Jin, Euncho Bae, Kwangwon Min, Hyeongseok Choi, Yunbae Kim, Seongkyoon Lee" << endl;
        cout << "read (LBA)         : Read data from (LBA)." << endl;
        cout << "write (LBA) (DATA) : Write (DATA) to (LBA)." << endl;
        cout << "fullread           : Read data from all LBAs." << endl;
        cout << "fullwrite (DATA)   : Write (DATA) to all LBAs" << endl;
        cout << "testscript         : Execute the predefined test script. See documentation for details." << endl;
        cout << "help               : Show usage instructions for all available commands." << endl;
        cout << "exit               : Exit the program." << endl;
        cout << "Note               : INVALID COMMAND will be shown if the input is unrecognized." << endl;
    }

    void read(int lba) {
        logger.print("testShell.read()", "read command called");

        if (lba < 0 || lba > 99) {
            testShellStringManager.printErrorReadResult();
            return;
        }
        ssd->read(lba);
        string result = readOutputFile();
        if (result == "ERROR") testShellStringManager.printErrorReadResult();
        else testShellStringManager.printSuccessReadResult(result, lba);
    }

    void fullRead() {
        logger.print("testShell.fullRead()", "full read command called");

        for (int lba = LBA_START_ADDRESS; lba <= LBA_END_ADDRESS; lba++) {
            ssd->read(lba);
            string result = readOutputFile();
            if (result == "ERROR") {
                testShellStringManager.printErrorReadResult();
                break;
            }
            testShellStringManager.printSuccessReadResult(result, lba);
        }
    }

    void write(int lba, string data)
    {
        logger.print("testShell.write()", "write command called");
        ssd->write(lba, data);
        if (isCmdExecuteError(readOutputFile())) {
            testShellStringManager.printErrorWriteResult();
            return;
        }
        testShellStringManager.printSuccessWriteResult();
    }

    void fullWrite(string data) {
        logger.print("testShell.fullWrite()", "full write command called");
        for (int lba = LBA_START_ADDRESS; lba <= LBA_END_ADDRESS; lba++) {
            ssd->write(lba, data);
            if (isCmdExecuteError(readOutputFile())) {
                testShellStringManager.printErrorFullWriteResult();
                return;
            }
        }
        testShellStringManager.printSuccessFullWriteResult();
    }

    void fullWriteAndReadCompare() {
        logger.print("testShell.fullWriteAndReadCompare()", "full write and read compare command called");

        for (int lba = LBA_START_ADDRESS; lba <= LBA_END_ADDRESS; ++lba) {
            string writeData = testShellStringManager.getWriteDataInFullWriteAndReadCompareScript(lba);

            ssd->write(lba, writeData);
            ssd->read(lba);
            string readData = readOutputFile();

            if (readData != writeData) {
                testShellStringManager.printWriteReadMismatch(lba, writeData, readData);
                testShellStringManager.printScriptFailResult();
                return;
            }
        }
        testShellStringManager.printScriptPassResult();
    }

    void exit(void) {
        cout << "Set Exit Comannd...\n";
        isExitCmd = true;
    }
    bool isExit() const {
        return isExitCmd;
    }

    void writeReadAging() {
        logger.print("testShell.writeReadAging()", "write read aging command called");

        for (int i = 0; i < WRITE_READ_ITERATION; i++) {
            string randomString = getRandomHexString();
            string firstLBAResult = getWriteReadResult(0, randomString);
            string endLBAResult = getWriteReadResult(99, randomString);

            if (firstLBAResult != endLBAResult) {
                testShellStringManager.printScriptFailResult();
                return;
            }
        }
        testShellStringManager.printScriptPassResult();
    }

    virtual string getRandomHexString() {
        return testShellStringManager.generateRandomHexString();
    }

    void partialLBAWrite() {
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
            result.push_back(readOutputFile());
            ssd->read(0);
            result.push_back(readOutputFile());
            ssd->read(3);
            result.push_back(readOutputFile());
            ssd->read(1);
            result.push_back(readOutputFile());
            ssd->read(2);
            result.push_back(readOutputFile());

            auto firstData = result[0];
            result.erase(result.begin());
            for (auto nextData : result) {
                if (firstData != nextData) {
                    testShellStringManager.printScriptFailResult();
                    return;
                }
            }
        }
        testShellStringManager.printScriptPassResult();
    }

    void eraseWithSize(unsigned int lba, unsigned int size) {
        logger.print("testShell.eraseWithSize()", "erase with size command called");

        if (!isValidEraseWithSizeArgument(lba, size)) {
            printEraseResult("Erase", "ERROR");
            return;
        }
        string result = erase(lba, size);
        printEraseResult("Erase", result);
    }

    void eraseWithRange(unsigned int startLba, unsigned int endLba) {
        logger.print("testShell.eraseWithRange()", "erase with range command called");

        if (!isValidLbaRange(startLba, endLba)) {
            printEraseResult("Erase Range", "ERROR");
            return;
        }
        const unsigned int size = endLba - startLba + 1;
        string result = erase(startLba, size);
        printEraseResult("Erase Range", result);
    }

    void eraseAndWriteAging(void) {
        logger.print("testShell.eraseAndWriteAging()", "erase and write aging command called");

        const int eraseUnitSize = 2;
        const int maxAgingCnt = 30;
        ssd->erase(0, eraseUnitSize);
        if (isCmdExecuteError(readOutputFile())) {
            cout << "FAIL\n";
            return;
        }

        for (int loopCnt = 0; loopCnt < maxAgingCnt; loopCnt++) {
            for (int lba = 2; lba < LBA_END_ADDRESS; lba += eraseUnitSize) {
                vector<string> result;
                ssd->write(lba, testShellStringManager.generateRandomHexString());
                result.push_back(readOutputFile());
                ssd->write(lba, testShellStringManager.generateRandomHexString());
                result.push_back(readOutputFile());
                ssd->erase(lba, eraseUnitSize);
                result.push_back(readOutputFile());

                for (auto data : result) {
                    if (isCmdExecuteError(data)) {
                        cout << "FAIL\n";
                        return;
                    }
                }
            }
            cout << "PASS\n";
        }
    }
    static const int WRITE_READ_ITERATION = 200;

private:
    SSD* ssd;
    Logger logger;
    TestShellStringManager testShellStringManager;

    bool isExitCmd{ false };

    const int LBA_START_ADDRESS = 0;
    const int LBA_END_ADDRESS = 99;

    const string ERASE_ERROR_MESSAGE = "[Erase] ERROR";
    const string ERASE_SUCCESS_MESSAGE = "[Erase] Done";

    bool isArgumentSizeValid(const string& cmd, int argsSize) {
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

    virtual string readOutputFile() {
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

    string getWriteReadResult(int lba, string input) {
        ssd->write(lba, input);
        ssd->read(lba);
        string result = readOutputFile();
        return result;
    }

    vector<string> tokenize(const string& input) {
        vector<string> tokens;
        istringstream iss(input);
        string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        return tokens;
    }

    bool isValidCommand(const string& cmd) const {
        static const unordered_set<string> valid = {
            "read", "write", "exit", "help", "fullread", "fullwrite",
            "testscript", "1_", "1_FullWriteAndReadCompare",
            "2_","2_PartialLBAWrite",
            "3_", "3_WriteReadAging",
            "erase","erase_range",
            "4_", "4_EraseAndWriteAging"
        };
        return valid.count(cmd) > 0;
    }
    string erase(unsigned int lba, unsigned int size) {
        const int maxEraseSize = 10;
        int currentLba = lba;
        for (int remainedSize = size; remainedSize > 0;) {
            int chunkSize = min(maxEraseSize, remainedSize);
            ssd->erase(currentLba, chunkSize);
            remainedSize -= chunkSize;
            currentLba += chunkSize;
            if (readOutputFile() == "ERROR") {
                return "ERROR";
            }
        }
        return "Done";
    }
    bool isValidLbaRange(unsigned int startLba, unsigned int endLba)
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
    bool isValidEraseWithSizeArgument(unsigned int lba, unsigned int size) {
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

    void printEraseResult(const string header, const string result)
    {
        cout << "[" << header << "] " << result << "\n";
    }

    bool isCmdExecuteError(const string result) const {
        return result == "ERROR";
    }
};

class MockTestShell : public TestShell {
public:
    MockTestShell(SSD* ssd) : TestShell(ssd) {}
    MOCK_METHOD(void, help, (), ());
    MOCK_METHOD(string, readOutputFile, (), ());
    MOCK_METHOD(string, getRandomHexString, (), ());
};
