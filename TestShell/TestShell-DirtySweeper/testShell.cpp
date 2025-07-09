#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <windows.h>
#include <shellapi.h>
#include <cstdlib>
#include <ctime>
#include "gmock/gmock.h"

using namespace std;

class SSD {
public:
    virtual void read(int lba) = 0;
    virtual void write(int lba, string data) = 0;
    virtual void erase(unsigned int lba, unsigned int size) = 0;
};

class SsdHelpler : public SSD {
public:
    string buildCommandLine(string rw, int lba, string data = "") {
        string cmdLine = rw + " " + std::to_string(lba);
        if (rw == "W") cmdLine = cmdLine + " " + data;
        return cmdLine;
    }

    void executeCommandLine(std::string commandLine) {
        const std::string filePath = "..\\..\\SSD\\x64\\Release\\ssd.exe";
        const std::string workingDir = "..\\..\\SSD\\x64\\Release";

        std::string fullCommand = "\"" + filePath + "\" " + commandLine;

        STARTUPINFOA si = { sizeof(STARTUPINFOA) };
        PROCESS_INFORMATION pi;

        BOOL success = CreateProcessA(
            nullptr,
            &fullCommand[0],        // commandLine (비 const)
            nullptr, nullptr, FALSE,
            0,
            nullptr,
            workingDir.c_str(),     // working directory 명시
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
        string commandLine = buildCommandLine("R", lba);
        executeCommandLine(commandLine);
    }

    void write(int lba, string data) override {
        string commandLine = buildCommandLine("W", lba, data);
        executeCommandLine(commandLine);
    }
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
            if (args.size() < 1) {
                std::cout << "INVALID COMMAND\n";
                return;
            }
            int lba = stoi(args[0]);
            this->read(lba);
            return;
        }

        if (cmd == "fullread") {
            this->fullRead();
            return;
        }

        if (cmd == "write") {
            if (args.size() < 2) {
                std::cout << "INVALID COMMAND\n";
                return;
            }
            int lba = stoi(args[0]);
            std::string data = args[1];
            this->write(lba, data);
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

        if (cmd == "fullwrite") {
            std::string data = args[0];
            this->fullWrite(data);
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

        std::cout << "INVALID COMMAND\n";
    }

    void processInput(const std::string& input) {
        auto tokens = tokenize(input);
        if (tokens.empty()) {
            return;
        }

        const std::string& cmd = tokens[0];
        std::vector<std::string> args(tokens.begin() + 1, tokens.end());

        if (!isValidCommand(cmd)) {
            std::cout << "INVALID COMMAND\n";
            return;
        }

        executeCommand(cmd, args);
    }

    void help() {
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
        if (lba < 0 || lba > 99) throw std::exception();
        ssd->read(lba);
        std::string result = readOutputFile();
        if (result == "ERROR") printErrorReadResult(result);
        else printSuccessReadResult(result, lba);
    }

    void fullRead() {
        for (int lba = LBA_START_ADDRESS; lba <= LBA_END_ADDRESS; lba++) {
            ssd->read(lba);
            std::string result = readOutputFile();
            if (result == "ERROR") {
                printErrorReadResult(result);
                break;
            }
            printSuccessReadResult(result, lba);
        }
    }

    string write(int lba, string data)
    {
        ssd->write(lba, data);
        string result = readOutputFile();
        if (result == "ERROR") {
            printErrorWriteResult();
            return WRITE_ERROR_MESSAGE;
        }

        printSuccessWriteResult();
        return WRITE_SUCCESS_MESSAGE;
    }

    string fullWrite(string data)
    {
        string totalResult = "";
        for (int lba = LBA_START_ADDRESS; lba <= LBA_END_ADDRESS; lba++) {
            ssd->write(lba, data);
            string currentResult = readOutputFile();
            if (currentResult == "ERROR") {
                totalResult += WRITE_ERROR_MESSAGE;
                printErrorWriteResult();
                break;
            }
            totalResult += WRITE_SUCCESS_MESSAGE + "\n";
            printSuccessWriteResult();
        }
        return totalResult;
    }

    std::string getWriteDataInFullWriteAndReadCompareScript(int lba){
        std::string evenData = "0xAAAABBBB";
        std::string oddData = "0xCCCCDDDD";
        return (lba / 5 % 2 == 0) ? evenData : oddData;
    }

    void fullWriteAndReadCompare() {
        for (int lba = LBA_START_ADDRESS; lba <= LBA_END_ADDRESS; ++lba) {
            std::string writeData = getWriteDataInFullWriteAndReadCompareScript(lba);

            ssd->write(lba, writeData);
            ssd->read(lba);
            std::string readData = readOutputFile();

            if (readData != writeData) {
                std::cout << "[Mismatch] LBA " << lba << " Expected: " << writeData << " Got: " << readData << "\n";
            }
        }
    }

    void exit(void) {
        std::cout << "Set Exit Comannd...\n";
        isExitCmd = true;
    }
    bool isExit() const {
        return isExitCmd;
    }

    void writeReadAging() {
        for (int i = 0; i < WRITE_READ_ITERATION; i++) {
            string randomString = generateRandomHexString();
            ssd->write(0, randomString);
            ssd->read(0);
            string firstLBAResult = readOutputFile();
            ssd->write(99, randomString);
            ssd->read(99);
            string endLBAResult = readOutputFile();

            if (firstLBAResult != endLBAResult) {
                cout << "FAIL";
                return;
            }
        }
        cout << "PASS";
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

    void erase(unsigned int lba, unsigned int size)
    {
        // exception 
        if (lba > LBA_END_ADDRESS) {
            throw::std::exception();
        }

        if (size < 1 || size > 100) {
            throw::std::exception();
        }
        if (lba + size > LBA_END_ADDRESS + 1) {
            throw::std::exception();
        }

        bool isFailed = false;
        const int maxEraseCntForSsd = 10;
        const int maxRepeatCnt = size / maxEraseCntForSsd;
        const int remainedSize = size % maxEraseCntForSsd;
        int startLba = lba;

        for (int cnt = 0; cnt < maxRepeatCnt; cnt++) {
            ssd->erase(startLba, maxEraseCntForSsd);
            startLba += maxEraseCntForSsd;
            if (readOutputFile() == "ERROR") {
                isFailed = true;
                break;
            }
        }
        
        if (!isFailed && remainedSize > 0) {
            ssd->erase(startLba, remainedSize);
            isFailed = ("ERROR" == readOutputFile());
        }

        if (isFailed) {
            printEraseResult("Erase", "ERROR");
        }
        else {
            printEraseResult("Erase", "Done");
        }
    }
    void eraseRange(unsigned int startLba, unsigned int endLba)
    {
        // exception 
        if (startLba < LBA_START_ADDRESS || startLba > LBA_END_ADDRESS) {
            throw::std::exception();
        }

        if (endLba < LBA_START_ADDRESS || endLba > LBA_END_ADDRESS) {
            throw::std::exception();
        }

        if (startLba > endLba) {
            throw::std::exception();
        }

        const unsigned int size = endLba - startLba + 1;

        bool isFailed = false;
        const int maxEraseCntForSsd = 10;
        const int maxRepeatCnt = size / maxEraseCntForSsd;
        const int remainedSize = size % maxEraseCntForSsd;

        for (int cnt = 0; cnt < maxRepeatCnt; cnt++) {
            ssd->erase(startLba, maxEraseCntForSsd);
            startLba += maxEraseCntForSsd;
            if (readOutputFile() == "ERROR") {
                isFailed = true;
                break;
            }
        }

        if (!isFailed && remainedSize > 0) {
            ssd->erase(startLba, remainedSize);
            isFailed = ("ERROR" == readOutputFile());
        }

        if (isFailed) {
            printEraseResult("Erase Range", "ERROR");
        }
        else {
            printEraseResult("Erase Range", "Done");
        }
    }

private:
    SSD* ssd;
    bool isExitCmd{ false };

    const int LBA_START_ADDRESS = 0;
    const int LBA_END_ADDRESS = 99;

    const string WRITE_ERROR_MESSAGE = "[Write] ERROR";
    const string WRITE_SUCCESS_MESSAGE = "[Write] Done";
    const string ERASE_ERROR_MESSAGE = "[Erase] ERROR";
    const string ERASE_SUCCESS_MESSAGE = "[Erase] Done";

    virtual std::string readOutputFile() {
        std::ifstream file("..\\..\\SSD\\x64\\Release\\ssd_output.txt");

        if (!file.is_open()) throw std::exception();

        std::ostringstream content;
        std::string line;
        while (std::getline(file, line)) {
            content << line;
        }
        std::string result = content.str();

        return result;
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
            "3_", "3_WriteReadAging"
        };
        return valid.count(cmd) > 0;
    }

    void printErrorReadResult(std::string result) {
        std::cout << "[Read] " << result << "\n";
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