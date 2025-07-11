#include "ssd.h"

using namespace std;

void SSDDriver::read(int lba) {
    logger.print("SSDDriver.read()", "Reading LBA: " + to_string(lba));
    string commandLine = buildCommandLine("R", lba);
    executeCommandLine(commandLine);
}

void SSDDriver::write(int lba, string data) {
    logger.print("SSDDriver.write()", "Writing to LBA: " + to_string(lba) + " with data: " + data);
    string commandLine = buildCommandLine("W", lba, data);
    executeCommandLine(commandLine);
}

void SSDDriver::erase(unsigned int lba, unsigned size) {
    logger.print("SSDDriver.erase()", "Erasing LBA: " + to_string(lba) + " with size: " + to_string(size));
    string commandLine = buildCommandLine("E", lba, to_string(size));
    executeCommandLine(commandLine);
}

void SSDDriver::flushSsdBuffer(void) {
    logger.print("SSDDriver.flushSsdBuffer()", "Flush");
    string commandLine = buildCommandLine("F");
    executeCommandLine(commandLine);
}

string SSDDriver::buildCommandLine(string cmd, int lba, string data) {
    if (cmd == "F") return cmd;
    string cmdLine = cmd + " " + to_string(lba);
    if (cmd == "W" || cmd == "E") cmdLine = cmdLine + " " + data;
    return cmdLine;
}

void SSDDriver::executeCommandLine(string commandLine) {
    FileAccessor::GetInstance()->executeSsdCommandLine(commandLine);
}
