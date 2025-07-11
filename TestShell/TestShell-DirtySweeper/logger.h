#pragma once
#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>
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

using namespace std;
class Logger {
public:
    static Logger& GetInstance();
    void print(const string& sender, const string& message);

private:
    Logger() = default;
    Logger(const Logger& c) = delete;
    Logger& operator=(const Logger&) = delete;
    const string LOG_FILE_NAME = "latest.log";
    const size_t MAX_LOG_FILE_SIZE = 10 * 1024; // 10KB

    string currentDateTime();
    string formatForFileName(tm tmStruct);
    string padRight(const string& str, size_t width);
    size_t getFileSize(const string& path);
    void rotateIfNeeded();
    void handleUntilLogFiles();
    time_t getLastWriteTime(const string& path);
};