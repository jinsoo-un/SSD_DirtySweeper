#include <sstream>
#include <vector>
#include <unordered_set>
#include <windows.h>
#include <shellapi.h>
#include <direct.h>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <io.h>
#include "gmock/gmock.h"
#include "testShell_string_manager.h"
#include "logger.h"
#include "ssd.h"

using namespace std;

void SsdHelpler::read(int lba) {
    logger.print("SsdHelpler.read()", "Reading LBA: " + to_string(lba));
    string commandLine = buildCommandLine("R", lba);
    executeCommandLine(commandLine);
}

void SsdHelpler::write(int lba, string data) {
    logger.print("SsdHelpler.write()", "Writing to LBA: " + to_string(lba) + " with data: " + data);
    string commandLine = buildCommandLine("W", lba, data);
    executeCommandLine(commandLine);
}

void SsdHelpler::erase(unsigned int lba, unsigned size) {
    logger.print("SsdHelpler.erase()", "Erasing LBA: " + to_string(lba) + " with size: " + to_string(size));
    string commandLine = buildCommandLine("E", lba, to_string(size));
    executeCommandLine(commandLine);
}

string SsdHelpler::buildCommandLine(string cmd, int lba, string data) {
    string cmdLine = cmd + " " + to_string(lba);
    if (cmd == "W" || cmd == "E") cmdLine = cmdLine + " " + data;
    return cmdLine;
}

void SsdHelpler::executeCommandLine(string commandLine) {

    char modulePath[MAX_PATH];
    GetModuleFileNameA(NULL, modulePath, MAX_PATH);

    string shellFullPath(modulePath);
    size_t lastSlash = shellFullPath.find_last_of("\\/");
    string shellDir = (lastSlash != string::npos) ? shellFullPath.substr(0, lastSlash) : ".";

    // ssd.exe 경로: shellDir 기준으로 상대 위치
    string ssdRelativePath = shellDir + "\\..\\..\\..\\SSD\\x64\\Release\\ssd.exe";
    string workingDirRelative = shellDir + "\\..\\..\\..\\SSD\\x64\\Release";

    // 절대경로로 변환
    char absSsdPath[MAX_PATH];
    _fullpath(absSsdPath, ssdRelativePath.c_str(), MAX_PATH);

    char absWorkingDir[MAX_PATH];
    _fullpath(absWorkingDir, workingDirRelative.c_str(), MAX_PATH);

    string fullCommand = "\"" + string(absSsdPath) + "\" " + commandLine;

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi;

    BOOL success = CreateProcessA(
        NULL,
        &fullCommand[0],  // 반드시 non-const!
        NULL, NULL, FALSE,
        0,
        NULL,
        absWorkingDir,
        &si, &pi
    );

    if (!success) {
        cerr << "CreateProcess failed with error: " << GetLastError() << endl;
        return;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}
