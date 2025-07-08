#include <string>
#include <vector>
#include <iostream>

using namespace std;

class CommandExecutor {
public:
    void executeCommand(const string& cmd, const vector<string>& args) {
        if (cmd == "read") {
            if (args.size() < 1) {
                std::cout << "INVALID COMMAND\n";
                return;
            }
            int lba = stoi(args[0]);
            read(lba);
            return;
        }

        if (cmd == "write") {
            if (args.size() < 2) {
                std::cout << "INVALID COMMAND\n";
                return;
            }
            int lba = stoi(args[0]);
            std::string data = args[1];
            write(lba, data);
            return;
        }

        if (cmd == "exit") {
            exit();
            return;
        }

        if (cmd == "help") {
            help();
            return;
        }

        if (cmd == "fullread") {
            fullRead();
            return;
        }

        if (cmd == "fullwrite") {
            fullWrite();
            return;
        }

        if (cmd == "testscript") {
            testScript();
            return;
        }

        std::cout << "INVALID COMMAND\n";
	}

protected:
    virtual void read(int lba) = 0;
    virtual void write(int lba, const string& data) = 0;
    virtual void exit() = 0;
    virtual void help() = 0;
    virtual void fullRead() = 0;
    virtual void fullWrite() = 0;
    virtual void testScript() = 0;
};