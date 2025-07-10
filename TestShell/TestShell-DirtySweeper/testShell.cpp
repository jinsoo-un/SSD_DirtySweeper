#include "testShell.h"
#include "command.h"

using namespace std;

string TestShell::executeCommand(const string& cmd, const vector<string>& args) {

    const string INVALID_COMMAND = "INVALID COMMAND";

    if (cmd == "read") {
        int lba = stoi(args[0]);
        ReadCommand readCommand(ssd, lba);
        return readCommand.execute();
        //return read(lba);
    }

    if (cmd == "fullread") {
        FullReadCommand fullReadCommand(ssd);
        return fullReadCommand.execute();
    }

    if (cmd == "write") {
        int lba = stoi(args[0]);
        string data = args[1];
        WriteCommand writeCommand(ssd, lba, data);
        return writeCommand.execute();
        //return write(lba, data);
    }

    if (cmd == "fullwrite") {
        string data = args[0];
        FullWriteCommand fullWriteCommand(ssd, data);
        return fullWriteCommand.execute();
        //return fullWrite(data);
    }

    if (cmd == "help") {
        //return help();
        return HelpCommand(ssd).execute();
    }

    if (cmd == "exit") {
        isExitCmd = true;
        //return exit();
        return ExitCommand(ssd).execute();
    }

    if (cmd == "1_" || cmd == "1_FullWriteAndReadCompare") {
        return FullWriteAndReadCompareCommand(ssd).execute();
    }

    if (cmd == "2_" || cmd == "2_PartialLBAWrite") {
        return PartialLBAWriteCommand(ssd).execute();
    }
    if (cmd == "3_" || cmd == "3_WriteReadAging") {
        return WriteReadAgingCommand(ssd).execute();
    }

    if (cmd == "erase") {
        if (args.size() != 2) {
            return INVALID_COMMAND;
        }
        int lba = stoi(args[0]);
        int size = stoi(args[1]);
        return EraseWithSizeCommand(ssd, lba, size).execute();
        //return eraseWithSize(lba, size);
    }

    if (cmd == "erase_range") {
        if (args.size() != 2) {
            return INVALID_COMMAND;
        }
        int startLba = stoi(args[0]);
        int endLba = stoi(args[1]);
        return EraseWithRangeCommand(ssd, startLba, endLba).execute();
        //return eraseWithRange(startLba, endLba);
    }
    if (cmd == "4_" || cmd == "4_EraseAndWriteAging") {
        return EraseAndWriteAgingCommand(ssd).execute();
        //return eraseAndWriteAging();
    }

    if (cmd == "flush") {
        //return flushSsdBuffer();
        return FlushCommand(ssd).execute();
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


bool TestShell::isExit() const {
    return isExitCmd;
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
