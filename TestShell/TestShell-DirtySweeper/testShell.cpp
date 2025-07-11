#include "testShell.h"
#include "command.h"

using namespace std;

string TestShell::executeCommand(const string& cmd, const vector<string>& args) {
    CommandFactory factory;
    auto commandPtr = factory.getCommand(ssd, cmd, args);
    return commandPtr->execute();
}

void TestShell::processInput(const string& input) {
    auto tokens = tokenize(input);
    if (tokens.empty()) {
        return;
    }

    const string& cmd = tokens[0];
    vector<string> args(tokens.begin() + 1, tokens.end());

    auto result = executeCommand(cmd, args);
    cout << result << endl;
}


bool TestShell::isExit() const {
    return CommandFactory::isExit();
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


