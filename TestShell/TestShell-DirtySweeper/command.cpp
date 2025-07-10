#include "command.h"

string ReadCommand::execute() {
    Logger::GetInstance().print("testShell.read()", "read command called");
    
    if (lba < 0 || lba > 99) {
        return TestShellOutputManager::GetInstance().getErrorReadResult();
    }

    ssd->read(lba);

    string result = FileAccessor::GetInstance()->readOutputFile();
    if (result == "ERROR")  return TestShellOutputManager::GetInstance().getErrorReadResult();
    return TestShellOutputManager::GetInstance().getSuccessReadResult(result, lba);
}

string FullReadCommand::execute() {
    string result = "";
    Logger::GetInstance().print("testShell.fullRead()", "full read command called");

    for (int lba = LBA_START_ADDRESS; lba <= LBA_END_ADDRESS; lba++) {
        ssd->read(lba);
        string outputResult = FileAccessor::GetInstance()->readOutputFile();
        if (outputResult == "ERROR") {
            result += TestShellOutputManager::GetInstance().getErrorReadResult();
            return result;
        }
        result += TestShellOutputManager::GetInstance().getSuccessReadResult(outputResult, lba);
        result += "\n";
    }
    return result;
}