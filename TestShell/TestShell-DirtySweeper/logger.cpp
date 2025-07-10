#include "gmock/gmock.h"
#include "testShell_output_manager.h"
#include "logger.h"

using namespace std;

Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}
void Logger::print(const string& sender, const string& message) {
    rotateIfNeeded();

    ofstream logFile(LOG_FILE_NAME, ios::app);
    if (!logFile.is_open()) return;

    logFile << "[" << currentDateTime() << "] ";
    logFile << padRight(sender, 30) << ": " << message << "\n";
}

string Logger::currentDateTime() {
    time_t now = time(nullptr);
    tm tmStruct;
    localtime_s(&tmStruct, &now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%y.%m.%d %H:%M", &tmStruct);
    return buffer;
}

string Logger::formatForFileName(tm tmStruct) {
    ostringstream oss;
    oss << "until_"
        << put_time(&tmStruct, "%y%m%d_%Hh_%Mm_%Ss")
        << ".log";
    return oss.str();
}

string Logger::padRight(const string& str, size_t width) {
    if (str.length() >= width) return str;
    return str + string(width - str.length(), ' ');
}

size_t Logger::getFileSize(const string& path) {
    struct _stat st;
    if (_stat(path.c_str(), &st) != 0) return 0;
    return st.st_size;
}

void Logger::rotateIfNeeded() {
    size_t size = getFileSize(LOG_FILE_NAME);
    if (size < MAX_LOG_FILE_SIZE) return;

    // rename latest.log to until_YYMMDD_HHh_MMm_SSs.log
    time_t now = time(nullptr);
    tm tmStruct;
    localtime_s(&tmStruct, &now);
    string newFileName = formatForFileName(tmStruct);

    rename(LOG_FILE_NAME.c_str(), newFileName.c_str());

    handleUntilLogFiles();
}

void Logger::handleUntilLogFiles() {
    vector<string> untilLogs;

    _finddata_t fileinfo;
    intptr_t handle = _findfirst("until_*.log", &fileinfo);
    if (handle != -1) {
        do {
            untilLogs.push_back(fileinfo.name);
        } while (_findnext(handle, &fileinfo) == 0);
        _findclose(handle);
    }

    if (untilLogs.size() < 2) return;

    // find oldest file
    string oldest = untilLogs[0];
    time_t oldestTime = getLastWriteTime(oldest);

    for (const auto& f : untilLogs) {
        time_t t = getLastWriteTime(f);
        if (t < oldestTime) {
            oldestTime = t;
            oldest = f;
        }
    }

    // rename .log to .zip
    string zipName = oldest.substr(0, oldest.find_last_of('.')) + ".zip";
    rename(oldest.c_str(), zipName.c_str());
}

time_t Logger::getLastWriteTime(const string& path) {
    WIN32_FILE_ATTRIBUTE_DATA info;
    if (!GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &info)) return 0;

    FILETIME ft = info.ftLastWriteTime;
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    return static_cast<time_t>((ull.QuadPart - 116444736000000000ULL) / 10000000ULL);
}
