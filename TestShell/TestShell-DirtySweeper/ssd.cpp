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

void SsdHelpler::flushSsdBuffer(void) {
    logger.print("SsdHelpler.flushSsdBuffer()", "Flush");
    string commandLine = buildCommandLine("F");
    executeCommandLine(commandLine);
}

string SsdHelpler::buildCommandLine(string cmd, int lba, string data) {
    if (cmd == "F") return cmd;
    string cmdLine = cmd + " " + to_string(lba);
    if (cmd == "W" || cmd == "E") cmdLine = cmdLine + " " + data;
    return cmdLine;
}

void SsdHelpler::executeCommandLine(string commandLine) {
    FileAccessor::GetInstance()->executeSsdCommandLine(commandLine);
}
