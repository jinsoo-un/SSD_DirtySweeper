#include <string>
#include <iostream>
#include "testShell_string_manager.h"

void TestShellStringManager::printErrorReadResult() {
    cout << "[Read] ERROR\n";
}

void TestShellStringManager::printSuccessReadResult(string result, int lba) {
    cout << "[Read] LBA " << lba << " : " << result << "\n";
}

void TestShellStringManager::printSuccessWriteResult() {
    cout << "[Write] Done\n";
}

void TestShellStringManager::printErrorWriteResult() {
    cout << "[Write] ERROR\n";
}

void TestShellStringManager::printSuccessFullWriteResult() {
    cout << "[Full Write] Done\n";;
}

void TestShellStringManager::printErrorFullWriteResult() {
    cout << "[Full Write] ERROR\n";
}

void TestShellStringManager::printScriptFailResult() {
    cout << "FAIL\n";
}

void TestShellStringManager::printScriptPassResult() {
    cout << "PASS\n";
}

void TestShellStringManager::printWriteReadMismatch(int lba, string writeData, string readData) {
    cout << "[Mismatch] LBA " << lba << " Expected: " << writeData << " Got: " << readData << "\n";
}

void TestShellStringManager::printEraseErrorResult() {
    cout << "[Erase] ERROR\n";
}

void TestShellStringManager::printErasePassResult() {
    cout << "[Erase] Done\n";
}

void TestShellStringManager::printEraseRangeErrorResult() {
    cout << "[Erase Range] ERROR\n";
}

void TestShellStringManager::printEraseRangePassResult() {
    cout << "[Erase Range] Done\n";
}

void TestShellStringManager::printSuccessFlushResult() {
    cout << "[Flush] Done\n";
}

void TestShellStringManager::printErrorFlushResult() {
    cout << "[Flush] ERROR\n";
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
