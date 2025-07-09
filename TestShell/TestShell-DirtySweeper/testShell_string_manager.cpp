#include <string>
#include <iostream>
#include "testShell_string_manager.h"

void TestShellStringManager::printErrorReadResult() {
    std::cout << "[Read] ERROR\n";
}

void TestShellStringManager::printSuccessReadResult(std::string result, int lba) {
    std::cout << "[Read] LBA " << lba << " : " << result << "\n";
}
void TestShellStringManager::printSuccessWriteResult() {
    std::cout << "[Write] Done\n";
}
void TestShellStringManager::printErrorWriteResult() {
    std::cout << "[Write] ERROR\n";
}

void TestShellStringManager::printSuccessFullWriteResult() {
    std::cout << "[Full Write] Done\n";;
}
void TestShellStringManager::printErrorFullWriteResult() {
    std::cout << "[Full Write] ERROR\n";
}

void TestShellStringManager::printScriptFailResult() {
    std::cout << "FAIL\n";
}
void TestShellStringManager::printScriptPassResult() {
    std::cout << "PASS\n";
}
void TestShellStringManager::printWriteReadMismatch(int lba, std::string writeData, std::string readData) {
    std::cout << "[Mismatch] LBA " << lba << " Expected: " << writeData << " Got: " << readData << "\n";
}

std::string TestShellStringManager::generateRandomHexString() {
    static const char* hexDigits = "0123456789ABCDEF";

    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        seeded = true;
    }

    unsigned int value = (static_cast<unsigned int>(std::rand()) << 16) | std::rand();

    std::string result = "0x";
    for (int i = 7; i >= 0; --i) {
        int digit = (value >> (i * 4)) & 0xF;
        result += hexDigits[digit];
    }

    return result;
}
std::string TestShellStringManager::getWriteDataInFullWriteAndReadCompareScript(int lba) {
    std::string evenData = "0xAAAABBBB";
    std::string oddData = "0xCCCCDDDD";
    return (lba / 5 % 2 == 0) ? evenData : oddData;
}