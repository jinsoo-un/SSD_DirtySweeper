#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include "gmock/gmock.h"

using namespace std;

class CommandExecutor {
public:
    virtual void read(int lba) = 0;
    virtual void write(int lba, const std::string& data) = 0;
    virtual void exit() = 0;
    virtual void help() = 0;
    virtual void fullRead() = 0;
    virtual void fullWrite() = 0;
    virtual void testScript() = 0;
};

class SSD {
public:
	virtual void read(int lba) = 0;
};

class SSDMock : public SSD {
public:
	MOCK_METHOD(void, read, (int lba), (override));
};

class TestShell {
public:
    TestShell() : commandExecutor(nullptr) {}
    TestShell(SSD* ssd) : ssd{ ssd } {}

    void setExecutor(CommandExecutor* executor) {
        commandExecutor = executor;
    }

    void executeCommand(const std::string& cmd, const std::vector<std::string>& args) {
        if (cmd == "read") {
            if (args.size() < 1) {
                std::cout << "INVALID COMMAND\n";
                return;
            }
            int lba = stoi(args[0]);
            commandExecutor->read(lba);
            return;
        }

        if (cmd == "write") {
            if (args.size() < 2) {
                std::cout << "INVALID COMMAND\n";
                return;
            }
            int lba = stoi(args[0]);
            std::string data = args[1];
            commandExecutor->write(lba, data);
            return;
        }

        if (cmd == "exit") {
            commandExecutor->exit();
            return;
        }

        if (cmd == "help") {
            commandExecutor->help();
            return;
        }

        if (cmd == "fullread") {
            commandExecutor->fullRead();
            return;
        }

        if (cmd == "fullwrite") {
            commandExecutor->fullWrite();
            return;
        }

        if (cmd == "testscript") {
            commandExecutor->testScript();
            return;
        }

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

        if (commandExecutor == nullptr) {
            std::cout << "NO EXECUTOR SET\n";
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
        if (result == "ERROR") {
            std::cout << "[Read] " << readOutputFile() << "\n";
        }
        else {
            std::cout << "[Read] LBA " << lba << " : " << result << "\n";
        }
    }

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

private:
    CommandExecutor* commandExecutor;
	SSD* ssd;

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
};

class MockTestShell : public TestShell {
public:
	MockTestShell(SSD* ssd) : TestShell(ssd) {}
	MOCK_METHOD(void, help, (), ());
	MOCK_METHOD(std::string, readOutputFile, (), ());
};