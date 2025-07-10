#include <string>
#include <sstream>
#include "commandParam.h"
#include "file.h"

using std::string;


const int VALID_DATA_LENGTH = 10;

class CommandParser {
public:
    bool parseCommand(string command, commandParams& param) {
        if (!isValidCommand(command)) {
            file.updateOutput("ERROR");
            return false;
        }
        param = extractParams(command);

        return true;
    }

private:
    commandParams extractParams(string command)
    {
        commandParams param;
        std::istringstream iss(command);
        string arg;
        int cnt = 0;
        /* scan the command line input */
        while (iss >> arg) {
            cnt++;
            if (cnt == 1)
                param.op = arg;
            if (cnt == 2)
                param.addr = std::stoi(arg);
            if (cnt == 3) {
                if (param.op == "E") param.size = std::stoi(arg);
                else param.value = arg;
            }
        }
        return param;
    }

    bool isAddressOutOfRange(int address) {
        return address < MIN_ADDRESS || address >= MAX_ADDRESS;
    }

    bool isEraseOutOfRange(int address, int size) {
        if (address + size > MAX_ADDRESS)
            return true;
        if (size < 1 || size > 10)
            return true;
        return false;
    }

    bool isValidCommand(string command) {
        std::istringstream iss(command);
        string arg;
        int cnt = 0;
        int tmpAddr = 0;
        bool isErase = false;

        while (iss >> arg) {
            cnt++;
            if (cnt == 1) {
                if (!isValidOp(arg)) return false;
                if (arg == "E") isErase = true;
            }
            else if (cnt == 2) {
                if (isAddressOutOfRange(stoi(arg))) return false;
                if (isErase) tmpAddr = stoi(arg);
            }
            else if (cnt == 3) {
                if (isErase) {
                    if (isEraseOutOfRange(tmpAddr, stoi(arg)))
                        return false;
                    else
                        continue;
                }
                if (!isValidWriteData(arg)) return false;
            }
            else
                return false;
        }

        return true;
    }

    bool isValidOp(string arg) {
        if (arg != "R" && arg != "W" && arg != "E" && arg != "F")
            return false;
        return true;
    }

    bool isValidWriteData(const std::string& str) {
        if (str.substr(0, 2) != "0x") return false;

        int length;
        for (length = 2; length < str.size(); length++) {
            char ch = static_cast<unsigned char>(str[length]);

            if (!(isNumber(ch) || isHexCharacter(ch))) { return false; }

        }

        if (length != VALID_DATA_LENGTH) { return false; }

        return true;
    }

    bool isHexCharacter(char ch) {
        return ((ch >= 'A') && (ch <= 'F'));
    }

    bool isNumber(char ch) {
        return ((ch >= '0') && (ch <= '9'));
    }

    FileControl& file = FileControl::get_instance();
};
