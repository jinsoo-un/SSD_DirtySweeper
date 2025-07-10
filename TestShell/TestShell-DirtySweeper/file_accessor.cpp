#include "file_accessor.h"
FileAccessor& FileAccessor::GetInstance() {
    static FileAccessor instance;
    return instance;
}

string FileAccessor::readOutputFile() {
    // shell.exe�� ���� ��� ���ϱ�
    char modulePath[MAX_PATH];
    GetModuleFileNameA(NULL, modulePath, MAX_PATH);

    string shellFullPath(modulePath);
    size_t lastSlash = shellFullPath.find_last_of("\\/");
    string shellDir = (lastSlash != string::npos) ? shellFullPath.substr(0, lastSlash) : ".";

    // ssd_output.txt�� ��� ��� �� ���� ��� ��ȯ
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

    // ssd.exe ���: shellDir �������� ��� ��ġ
    string ssdRelativePath = shellDir + "\\..\\..\\..\\SSD\\x64\\Release\\ssd.exe";
    string workingDirRelative = shellDir + "\\..\\..\\..\\SSD\\x64\\Release";

    // �����η� ��ȯ
    char absSsdPath[MAX_PATH];
    _fullpath(absSsdPath, ssdRelativePath.c_str(), MAX_PATH);

    char absWorkingDir[MAX_PATH];
    _fullpath(absWorkingDir, workingDirRelative.c_str(), MAX_PATH);

    string fullCommand = "\"" + string(absSsdPath) + "\" " + commandLine;

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi;

    BOOL success = CreateProcessA(
        NULL,
        &fullCommand[0],  // �ݵ�� non-const!
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