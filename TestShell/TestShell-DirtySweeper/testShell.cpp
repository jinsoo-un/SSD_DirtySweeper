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
    void print(const std::string& sender, const std::string& message) {
        rotateIfNeeded();

        std::ofstream logFile(LOG_FILE_NAME, std::ios::app);
        if (!logFile.is_open()) return;

        logFile << "[" << currentDateTime() << "] ";
        logFile << padRight(sender, 30) << ": " << message << "\n";
    }

private:
    const std::string LOG_FILE_NAME = "latest.log";
    const size_t MAX_LOG_FILE_SIZE = 10 * 1024; // 10KB

    std::string currentDateTime() {
        std::time_t now = std::time(nullptr);
        std::tm tmStruct;
        localtime_s(&tmStruct, &now);
        char buffer[20];
        std::strftime(buffer, sizeof(buffer), "%y.%m.%d %H:%M", &tmStruct);
        return buffer;
    }

    std::string formatForFileName(std::tm tmStruct) {
        std::ostringstream oss;
        oss << "until_"
            << std::put_time(&tmStruct, "%y%m%d_%Hh_%Mm_%Ss")
            << ".log";
        return oss.str();
    }

    std::string padRight(const std::string& str, size_t width) {
        if (str.length() >= width) return str;
        return str + std::string(width - str.length(), ' ');
    }

    size_t getFileSize(const std::string& path) {
        struct _stat st;
        if (_stat(path.c_str(), &st) != 0) return 0;
        return st.st_size;
    }

    void rotateIfNeeded() {
        size_t size = getFileSize(LOG_FILE_NAME);
        if (size < MAX_LOG_FILE_SIZE) return;

        // rename latest.log to until_YYMMDD_HHh_MMm_SSs.log
        std::time_t now = std::time(nullptr);
        std::tm tmStruct;
        localtime_s(&tmStruct, &now);
        std::string newFileName = formatForFileName(tmStruct);

        std::rename(LOG_FILE_NAME.c_str(), newFileName.c_str());

        handleUntilLogFiles();
    }

    void handleUntilLogFiles() {
        std::vector<std::string> untilLogs;

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
        std::string oldest = untilLogs[0];
        time_t oldestTime = getLastWriteTime(oldest);

        for (const auto& f : untilLogs) {
            time_t t = getLastWriteTime(f);
            if (t < oldestTime) {
                oldestTime = t;
                oldest = f;
            }
        }

        // rename .log to .zip
        std::string zipName = oldest.substr(0, oldest.find_last_of('.')) + ".zip";
        std::rename(oldest.c_str(), zipName.c_str());
    }

    time_t getLastWriteTime(const std::string& path) {
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
        string cmdLine = cmd + " " + std::to_string(lba);
        if (cmd == "W" || cmd == "E") cmdLine = cmdLine + " " + data;
        return cmdLine;
    }

    void executeCommandLine(std::string commandLine) {

        char modulePath[MAX_PATH];
        GetModuleFileNameA(NULL, modulePath, MAX_PATH);

        std::string shellFullPath(modulePath);
        size_t lastSlash = shellFullPath.find_last_of("\\/");
        std::string shellDir = (lastSlash != std::string::npos) ? shellFullPath.substr(0, lastSlash) : ".";

        // ssd.exe 경로: shellDir 기준으로 상대 위치
        std::string ssdRelativePath = shellDir + "\\..\\..\\..\\SSD\\x64\\Release\\ssd.exe";
        std::string workingDirRelative = shellDir + "\\..\\..\\..\\SSD\\x64\\Release";

        // 절대경로로 변환
        char absSsdPath[MAX_PATH];
        _fullpath(absSsdPath, ssdRelativePath.c_str(), MAX_PATH);

        char absWorkingDir[MAX_PATH];
        _fullpath(absWorkingDir, workingDirRelative.c_str(), MAX_PATH);

        std::string fullCommand = "\"" + std::string(absSsdPath) + "\" " + commandLine;

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
            std::cerr << "CreateProcess failed with error: " << GetLastError() << std::endl;
            return;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    void read(int lba)  override {
		logger.print("SsdHelpler.read()", "Reading LBA: " + std::to_string(lba));
        string commandLine = buildCommandLine("R", lba);
        executeCommandLine(commandLine);
    }

    void write(int lba, string data) override {
		logger.print("SsdHelpler.write()", "Writing to LBA: " + std::to_string(lba) + " with data: " + data);
        string commandLine = buildCommandLine("W", lba, data);
        executeCommandLine(commandLine);
    }
    void erase(unsigned int lba, unsigned size) override {
		logger.print("SsdHelpler.erase()", "Erasing LBA: " + std::to_string(lba) + " with size: " + std::to_string(size));
        string commandLine = buildCommandLine("E", lba, std::to_string(size));
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

    void executeCommand(const std::string& cmd, const std::vector<std::string>& args) {
        

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
            std::string data = args[1];
            this->write(lba, data);
            return;
        }

        if (cmd == "fullwrite") {
            std::string data = args[0];
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
            writeReadAging();
            return;
        }

        if (cmd == "erase") {
            if (args.size() != 2) {
                std::cout << "INVALID COMMAND\n";
                return;
            }
            int lba = stoi(args[0]);
            int size = stoi(args[1]);
            this->eraseWithSize(lba, size);
            return;
        }

        if (cmd == "erase_range") {
            if (args.size() != 2) {
                std::cout << "INVALID COMMAND\n";
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
        std::cout << "INVALID COMMAND\n";
    }

    void processInput(const std::string& input) {
        auto tokens = tokenize(input);
        if (tokens.empty()) {
            return;
        }

        const std::string& cmd = tokens[0];
        std::vector<std::string> args(tokens.begin() + 1, tokens.end());

        if (isValidCommand(cmd) && isArgumentSizeValid(cmd, args.size())) {
            executeCommand(cmd, args);
            return;
        }

        std::cout << "INVALID COMMAND\n";
    }

    void help() {
		logger.print("testShell.help()", "help command called");

        std::cout << "Developed by: Team Members - Sooeon Jin, Euncho Bae, Kwangwon Min, Hyeongseok Choi, Yunbae Kim, Seongkyoon Lee" << std::endl;
        std::cout << "read (LBA)         : Read data from (LBA)." << std::endl;
        std::cout << "write (LBA) (DATA) : Write (DATA) to (LBA)." << std::endl;
        std::cout << "fullread           : Read data from all LBAs." << std::endl;
        std::cout << "fullwrite (DATA)   : Write (DATA) to all LBAs" << std::endl;
        std::cout << "testscript         : Execute the predefined test script. See documentation for details." << std::endl;
        std::cout << "help               : Show usage instructions for all available commands." << std::endl;
        std::cout << "exit               : Exit the program." << std::endl;
        std::cout << "Note               : INVALID COMMAND will be shown if the input is unrecognized." << std::endl;
    }

    void read(int lba) {
        logger.print("testShell.read()", "read command called");

        if (lba < 0 || lba > 99) {
            printErrorReadResult();
            return;
        }
        ssd->read(lba);
        std::string result = readOutputFile();
        if (result == "ERROR") printErrorReadResult();
        else printSuccessReadResult(result, lba);
    }

    void fullRead() {
        logger.print("testShell.fullRead()", "full read command called");

        for (int lba = LBA_START_ADDRESS; lba <= LBA_END_ADDRESS; lba++) {
            ssd->read(lba);
            std::string result = readOutputFile();
            if (result == "ERROR") {
                printErrorReadResult();
                break;
            }
            printSuccessReadResult(result, lba);
        }
    }

    string write(int lba, string data) {
		logger.print("testShell.write()", "write command called");

        ssd->write(lba, data);
        string result = readOutputFile();
        if (result == "ERROR") {
            printErrorWriteResult();
            return WRITE_ERROR_MESSAGE;
        }

        printSuccessWriteResult();
        return WRITE_SUCCESS_MESSAGE;
    }

    void fullWrite(string data) {
		logger.print("testShell.fullWrite()", "full write command called");

        for (int lba = LBA_START_ADDRESS; lba <= LBA_END_ADDRESS; lba++) {
            ssd->write(lba, data);
            string currentResult = readOutputFile();
            if (currentResult == "ERROR") {
                cout << "[Full Write] ERROR\n";
                return;
            }
        }
        cout << "[Full Write] Done\n";;
    }

    std::string getWriteDataInFullWriteAndReadCompareScript(int lba){
        std::string evenData = "0xAAAABBBB";
        std::string oddData = "0xCCCCDDDD";
        return (lba / 5 % 2 == 0) ? evenData : oddData;
    }

    void fullWriteAndReadCompare() {
		logger.print("testShell.fullWriteAndReadCompare()", "full write and read compare command called");

        for (int lba = LBA_START_ADDRESS; lba <= LBA_END_ADDRESS; ++lba) {
            std::string writeData = getWriteDataInFullWriteAndReadCompareScript(lba);

            ssd->write(lba, writeData);
            ssd->read(lba);
            std::string readData = readOutputFile();

            if (readData != writeData) {
                std::cout << "[Mismatch] LBA " << lba << " Expected: " << writeData << " Got: " << readData << "\n";
                std::cout << "FAIL\n";
                return;
            }
        }
        std::cout << "PASS\n";
    }

    void exit(void) {
        std::cout << "Set Exit Comannd...\n";
        isExitCmd = true;
    }
    bool isExit() const {
        return isExitCmd;
    }

    void writeReadAging() {
		logger.print("testShell.writeReadAging()", "write read aging command called");

        for (int i = 0; i < WRITE_READ_ITERATION; i++) {
            string randomString = generateRandomHexString();
            ssd->write(0, randomString);
            ssd->read(0);
            string firstLBAResult = readOutputFile();
            ssd->write(99, randomString);
            ssd->read(99);
            string endLBAResult = readOutputFile();

            if (firstLBAResult != endLBAResult) {
                cout << "FAIL\n";
                return;
            }
        }
        cout << "PASS\n";
    }

    virtual std::string generateRandomHexString() {
        static const char* hexDigits = "0123456789ABCDEF";

        static bool seeded = false;
        if (!seeded) {
            std::srand(static_cast<unsigned int>(std::time(nullptr)));
            seeded = true;
        }

        unsigned int value = (static_cast<unsigned int>(std::rand()) << 16) | std::rand();

        std::string result = "0x";
        for (int i = 7; i >= 0; --i) {
            int digit = (value >> (i * 4)) & 0xF;
            result += hexDigits[digit];
        }

        return result;
    }

    static const int WRITE_READ_ITERATION = 200;

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
                    std::cout << "FAIL\n";
                    return;
                }
            }
        }
        std::cout << "PASS\n";
    }

    void eraseWithSize(unsigned int lba, unsigned int size){
		logger.print("testShell.eraseWithSize()", "erase with size command called");

        if (!isValidEraseWithSizeArgument(lba,size)) {
            printEraseResult("Erase", "ERROR");
            return;
        }
        string result = erase(lba, size);
        printEraseResult("Erase", result);
    }

    void eraseWithRange(unsigned int startLba, unsigned int endLba){
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
        if (readOutputFile() == "ERROR") {
            std::cout << "FAIL\n";
            return;
        }

        for (int loopCnt = 0; loopCnt < maxAgingCnt; loopCnt++) {
            for (int lba = 2; lba < LBA_END_ADDRESS; lba += eraseUnitSize){
                vector<string> result;
                ssd->write(lba, generateRandomHexString());
                result.push_back(readOutputFile());
                ssd->write(lba, generateRandomHexString());
                result.push_back(readOutputFile());
                ssd->erase(lba, eraseUnitSize);
                result.push_back(readOutputFile());

                for (auto data : result) {
                    if (data == "ERROR") {
                        std::cout << "FAIL\n";
                        return;
                    }
                }
            }
            std::cout << "PASS\n";
        }
    }

private:
    SSD* ssd;
	Logger logger;
    bool isExitCmd{ false };

    const int LBA_START_ADDRESS = 0;
    const int LBA_END_ADDRESS = 99;

    const string WRITE_ERROR_MESSAGE = "[Write] ERROR";
    const string WRITE_SUCCESS_MESSAGE = "[Write] Done";
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

    virtual std::string readOutputFile() {
        // shell.exe의 절대 경로 구하기
        char modulePath[MAX_PATH];
        GetModuleFileNameA(NULL, modulePath, MAX_PATH);

        std::string shellFullPath(modulePath);
        size_t lastSlash = shellFullPath.find_last_of("\\/");
        std::string shellDir = (lastSlash != std::string::npos) ? shellFullPath.substr(0, lastSlash) : ".";

        // ssd_output.txt의 상대 경로 → 절대 경로 변환
        std::string relativePath = shellDir + "\\..\\..\\..\\SSD\\x64\\Release\\ssd_output.txt";

        char absPath[MAX_PATH];
        _fullpath(absPath, relativePath.c_str(), MAX_PATH);

        std::ifstream file(absPath);
        if (!file.is_open()) {
            std::cerr << "Failed to open output file: " << absPath << std::endl;
            throw std::exception();
        }

        std::ostringstream content;
        std::string line;
        while (std::getline(file, line)) {
            content << line;
        }

        return content.str();
    }

    std::vector<std::string> tokenize(const std::string& input) {
        std::vector<std::string> tokens;
        std::istringstream iss(input);
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        return tokens;
    }

    bool isValidCommand(const std::string& cmd) const {
        static const std::unordered_set<std::string> valid = {
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
    void printErrorReadResult() {
        std::cout << "[Read] ERROR\n";
    }

    void printSuccessReadResult(std::string result, int lba) {
        std::cout << "[Read] LBA " << lba << " : " << result << "\n";
    }
    void printSuccessWriteResult() {
        std::cout << WRITE_SUCCESS_MESSAGE << "\n";
    }
    void printErrorWriteResult() {
        std::cout << WRITE_ERROR_MESSAGE << "\n";
    }
    void printEraseResult(const string header, const string result)
    {
        std::cout <<"["<< header<<"] "<< result << "\n";
    }
};

class MockTestShell : public TestShell {
public:
	MockTestShell(SSD* ssd) : TestShell(ssd) {}
	MOCK_METHOD(void, help, (), ());
	MOCK_METHOD(std::string, readOutputFile, (), ());
    MOCK_METHOD(std::string, generateRandomHexString, (), ());
};