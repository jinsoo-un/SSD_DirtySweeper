#include <iostream>
#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <shellapi.h>
#include "gmock/gmock.h"

using namespace std;

class CommandExecutor {
public:
    virtual std::string read() = 0;
    virtual std::string write() = 0;
    virtual std::string exit() = 0;
    virtual std::string help() = 0;
    virtual std::string fullRead() = 0;
    virtual std::string fullWrite() = 0;
    virtual std::string testScript() = 0;
};

class SSD {
public:
    virtual void read(int lba) = 0;
    virtual void write(int lba, string data) = 0;
    virtual string getResult() = 0;
};

class SsdHelpler : public SSD {
public:
    void read(int lba)  override {}
    void write(int lba, string data) override {
        // 1. set cli
        const string result = "";
        const string filePath = "./ssd.exe";
        const string writeCmd = "W";
        const string commandLine = writeCmd + std::to_string(lba) + data;

        // 2. ssd.exe 실행
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
    TestShell() : commandExecutor(nullptr) {}
    TestShell(SSD* ssd) : ssd{ ssd } {}

    void setExecutor(CommandExecutor* executor) {
        commandExecutor = executor;
    }

    std::string executeCommand(const std::string& cmd) {
        if (commandExecutor == nullptr) {
            return "NO EXECUTOR SET";
        }

        if (cmd == "read") return commandExecutor->read();
        if (cmd == "write") return commandExecutor->write();
        if (cmd == "exit") return commandExecutor->exit();
        if (cmd == "help") return commandExecutor->help();
        if (cmd == "fullread") return commandExecutor->fullRead();
        if (cmd == "fullwrite") return commandExecutor->fullWrite();
        if (cmd == "testscript") return commandExecutor->testScript();
        return "INVALID COMMAND";
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
        if (result == "") {
            return "[Write] Done";
        }
        return "[Write] ERROR";
    }

private:
    CommandExecutor* commandExecutor;
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