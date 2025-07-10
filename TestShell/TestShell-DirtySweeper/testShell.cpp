#include "testShell.h"
#include "command.h"

using namespace std;

string TestShell::executeCommand(const string& cmd, const vector<string>& args) {
    const string INVALID_COMMAND = "INVALID COMMAND";
    CommandFactory factory;
    auto commandPtr = factory.getCommand(ssd, cmd, args);
    if (commandPtr == nullptr) return INVALID_COMMAND;
    return commandPtr->execute();
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
    else if (cmd == "erase") {
        if (argsSize != 2) return false;
    }
    else if (cmd == "erase_range") {
        if (argsSize != 2) return false;
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
