#include <string>
#include <iostream>
#include "testShell_output_manager.h"

TestShellOutputManager& TestShellOutputManager::GetInstance() {
    static TestShellOutputManager instance;
    return instance;
}

string TestShellOutputManager::getErrorReadResult()
{
    return "[Read] ERROR";
}

string TestShellOutputManager::getSuccessReadResult(string result, int lba)
{
    return  "[Read] LBA " + std::to_string(lba) + " : " + result;
}

string TestShellOutputManager::getSuccessWriteResult() {
    return "[Write] Done";
}

string TestShellOutputManager::getErrorWriteResult() {
    return "[Write] ERROR";
}

string TestShellOutputManager::getSuccessFullWriteResult() {
   return "[Full Write] Done";
}

string TestShellOutputManager::getErrorFullWriteResult() {
    return "[Full Write] ERROR";
}

string TestShellOutputManager::getScriptFailResult() {
    return "FAIL";
}

string TestShellOutputManager::getScriptPassResult() {
    return "PASS";
}

string TestShellOutputManager::getWriteReadMismatch(int lba, string writeData, string readData) {
    return "[Mismatch] LBA " + to_string(lba) + " Expected: " +  writeData + " Got: " + readData;
}

string TestShellOutputManager::getEraseErrorResult() {
    return "[Erase] ERROR";
}

string TestShellOutputManager::getErasePassResult() {
    return "[Erase] Done";
}

string TestShellOutputManager::getEraseRangeErrorResult() {
    return "[Erase Range] ERROR";
}

string TestShellOutputManager::getEraseRangePassResult() {
    return "[Erase Range] Done";
}

string TestShellOutputManager::getSuccessFlushResult() {
    return "[Flush] Done";
}

string TestShellOutputManager::getErrorFlushResult() {
    return "[Flush] ERROR";
}



