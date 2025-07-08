#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <windows.h>
#include <shellapi.h>
#include "gmock/gmock.h"

using namespace std;

class SSD {
public:
	virtual void read(int lba) = 0;
    virtual void write(int lba, string data) = 0;
    virtual string getResult() = 0;
};

class SsdHelpler : public SSD {
public:
    string buildCommandLine(string rw, int lba, string data = "") {
        string cmdLine = rw + " " + std::to_string(lba);
        if (rw == "W") cmdLine = cmdLine + " " + data;
        return cmdLine;
    }

    void executeCommandLine(string& commandLine) {
        const string filePath = "./ssd.exe";
        HINSTANCE executeResult = ShellExecuteA( // ShellExecuteA는 ANSI 문자열용, ShellExecuteW는 유니코드용
            nullptr,                      // 부모 윈도우 핸들
            "open",                       // 수행할 작업 (예: "open", "runas")
            filePath.c_str(),             // 실행할 파일 경로
            commandLine.c_str(),           // 인자 문자열
            nullptr,                      // 시작 디렉토리
            SW_SHOWNORMAL                 // 윈도우 보여주기 상태
        );

        if (reinterpret_cast<long long>(executeResult) <= 32) {
            std::cerr << "Failed to launch: " << filePath << ". Error code: " << reinterpret_cast<long long>(executeResult) << std::endl;
            throw std::exception();
        }
    }

    void read(int lba)  override {
        string commandLine = buildCommandLine("R", lba);
        executeCommandLine(commandLine);
    }

    void write(int lba, string data) override {
        string commandLine = buildCommandLine("W", lba, data);
        executeCommandLine(commandLine);
    }
    string getResult() override {
        return "";
    }
};

class SSDMock : public SSD {
public:
	MOCK_METHOD(void, read, (int lba), (override));
	MOCK_METHOD(void, write, (int, string), (override));
	MOCK_METHOD(string, getResult, (), (override));
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

        // ��� �̱������� �ӽ� disabled
        //if (cmd == "exit") {
        //    this->exit();
        //    return;
        //}

        //if (cmd == "fullwrite") {
        //    this->fullWrite();
        //    return;
        //}

        //if (cmd == "testscript") {
        //    this->testScript();
        //    return;
        //}

        std::cout << "INVALID COMMAND\n";
    }

    void processInput(const std::string& input) {
        auto tokens = tokenize(input);
        if (tokens.empty()) {
            std::cout << "INVALID COMMAND\n";
            return;
        }

        const std::string& cmd = tokens[0];
        std::vector<std::string> args(tokens.begin() + 1, tokens.end());

        if (!isValidCommand(cmd)) {
            std::cout << "INVALID COMMAND\n";
            return;
        }

        executeCommand(cmd, args);  // commandExecutor는 항상 존재함
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
        string result = ssd->getResult();
        if (result == "ERROR") {
            return "[Write] ERROR";
        }
        return "[Write] Done";
    }

private:
    SSD* ssd;

    const int LBA_START_ADDRESS = 0;
    const int LBA_END_ADDRESS = 99;

    virtual std::string readOutputFile() {
        std::ifstream file("nand_output.txt");

        if (!file.is_open()) throw std::exception();

        std::ostringstream content;
        std::string line;
        while (std::getline(file, line)) {
            content << line;
        }
        std::string result = content.str();

        std::cout << result << "\n";
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
            "read", "write", "exit", "help", "fullread", "fullwrite", "testscript"
        };
        return valid.count(cmd) > 0;
    }

    void printErrorReadResult(std::string result) {
        std::cout << "[Read] " << result << "\n";
    }

    void printSuccessReadResult(std::string result, int lba) {
        std::cout << "[Read] LBA " << lba << " : " << result << "\n";
    }
};

class MockTestShell : public TestShell {
public:
	MockTestShell(SSD* ssd) : TestShell(ssd) {}
	MOCK_METHOD(void, help, (), ());
	MOCK_METHOD(std::string, readOutputFile, (), ());
};