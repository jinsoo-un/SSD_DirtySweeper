#include "file_accessor.h"
FileAccessor& FileAccessor::GetInstance() {
    static FileAccessor instance;
    return instance;
}

string FileAccessor::readOutputFile() {
    // shell.exe의 절대 경로 구하기
    char modulePath[MAX_PATH];
    GetModuleFileNameA(NULL, modulePath, MAX_PATH);

    string shellFullPath(modulePath);
    size_t lastSlash = shellFullPath.find_last_of("\\/");
    string shellDir = (lastSlash != string::npos) ? shellFullPath.substr(0, lastSlash) : ".";

    // ssd_output.txt의 상대 경로 → 절대 경로 변환
    string relativePath = shellDir + "\\..\\..\\..\\SSD\\x64\\Release\\ssd_output.txt";

    char absPath[MAX_PATH];
    _fullpath(absPath, relativePath.c_str(), MAX_PATH);

    ifstream file(absPath);
    if (!file.is_open()) {
        cerr << "Failed to open output file: " << absPath << endl;
        throw exception();
    }

    ostringstream content;
    string line;
    while (getline(file, line)) {
        content << line;
    }

    return content.str();
}

void FileAccessor::executeSsdCommandLine(string commandLine) {
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