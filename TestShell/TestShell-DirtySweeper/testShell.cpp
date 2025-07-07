#include <iostream>
#include <string>

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
	virtual void read(int address) = 0;
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

    void read(int address) {
        ssd->read(address);
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

private:
    CommandExecutor* commandExecutor;
	SSD* ssd;
};