#include <string>
#include <iostream>
#include "testShell_string_manager.h"

string TestShellStringManager::getErrorReadResult()
{
    return "[Read] ERROR";
}

string TestShellStringManager::getSuccessReadResult(string result, int lba)
{
    return  "[Read] LBA " + std::to_string(lba) + " : " + result;
}

string TestShellStringManager::getSuccessWriteResult() {
    return "[Write] Done";
}

string TestShellStringManager::getErrorWriteResult() {
    return "[Write] ERROR";
}

string TestShellStringManager::getSuccessFullWriteResult() {
   return "[Full Write] Done";
}

string TestShellStringManager::getErrorFullWriteResult() {
    return "[Full Write] ERROR";
}

string TestShellStringManager::getScriptFailResult() {
    return "FAIL";
}

string TestShellStringManager::getScriptPassResult() {
    return "PASS";
}

string TestShellStringManager::getWriteReadMismatch(int lba, string writeData, string readData) {
    return "[Mismatch] LBA " + to_string(lba) + " Expected: " +  writeData + " Got: " + readData;
}

string TestShellStringManager::getEraseErrorResult() {
    return "[Erase] ERROR";
}

string TestShellStringManager::getErasePassResult() {
    return "[Erase] Done";
}

string TestShellStringManager::getEraseRangeErrorResult() {
    return "[Erase Range] ERROR";
}

string TestShellStringManager::getEraseRangePassResult() {
    return "[Erase Range] Done";
}

string TestShellStringManager::generateRandomHexString() {
    static const char* hexDigits = "0123456789ABCDEF";

    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned int>(time(nullptr)));
        seeded = true;
    }

    unsigned int value = (static_cast<unsigned int>(rand()) << 16) | rand();

    string result = "0x";
    for (int i = 7; i >= 0; --i) {
        int digit = (value >> (i * 4)) & 0xF;
        result += hexDigits[digit];
    }

    return result;
}

string TestShellStringManager::getWriteDataInFullWriteAndReadCompareScript(int lba) {
    string evenData = "0xAAAABBBB";
    string oddData = "0xCCCCDDDD";
    return (lba / 5 % 2 == 0) ? evenData : oddData;
}

string TestShellStringManager::getNewLine() {
    return "\n";
}

