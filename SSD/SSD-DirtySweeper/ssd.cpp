#include <algorithm>
#include <vector>
#include "file.h"
#include "commandParam.h"
#include "commandParser.cpp"
#include "command.cpp"
#include "buffer.cpp"

using std::string;

// SSD Interface Class
class SSD {
public:
    virtual bool execute(string command) = 0;
    virtual bool executeWithParam(commandParams cmd) = 0;
    virtual int getAccessCount() = 0;
protected:
    FileControl& file = FileControl::get_instance();
};

class RealSSD : public SSD {
public:
    bool execute(string command) {
        commandParams currentCmd;
        if (parser.parseCommand(command, currentCmd) == false) return false;
        
        return executeWithParam(currentCmd);
    }

    bool executeWithParam(commandParams cmd) {
        command = factory.getCommand(cmd.op);
        if (command == nullptr) return false;

        UpdateAccessCount(cmd);

        return command->run(cmd.addr, cmd.value, cmd.size);
    }

    int getAccessCount() {
        return accessCount;
    }

private:
    void UpdateAccessCount(const commandParams& cmd)
    {
        if (cmd.op == "W") accessCount++;
        if (cmd.op == "E") accessCount += cmd.size;
    }

    int accessCount;

    CommandParser parser;
    SSDCommand* command = nullptr;
    SSDCommandFactory factory;
};

// SSD Proxy Class
class BufferedSSD : public SSD {
public:
    BufferedSSD() : ssd{ new RealSSD() } {}
    bool execute(string command) {
        commandParams currentCmd;
        if (parser.parseCommand(command, currentCmd) == false) return false;

        return executeWithParam(currentCmd);
    }
 
    bool executeWithParam(commandParams cmd) {
        string operation = cmd.op;
        if (operation == "R") return read(cmd);
        if (operation == "W") return write(cmd);
        if (operation == "E") return erase(cmd);
        if (operation == "F") {
            if (flushBuffer() == false) {
                file.updateOutput("ERROR");
            }
            else {
                file.updateOutput("");
            }
            return true;
        }
    }

    int getAccessCount() {
        return ssd->getAccessCount();
    }

private:
    // Buffered SSD methods
    bool read(commandParams cmd) {
        if (buffer.isEmpty())
            return ssd->executeWithParam(cmd);

        if (checkAndReadFromBuffer(cmd))
                return true;

        return ssd->executeWithParam(cmd);
    }

    bool write(const commandParams& cmd) {
        if (buffer.isFull()) {
            if (flushBuffer() == false) {
                file.updateOutput("ERROR");
            }
        }
        
        buffer.writeBuffer(cmd);
        CheckBufferCmdErasable(cmd);
  
        return true;        
    }

    bool erase(const commandParams& cmd) {
        if (buffer.isFull()) {
            flushBufferandAddCurrentCmd(cmd);
            return true;
        }
   
        int startAddr = cmd.addr;
        int endAddr = startAddr + cmd.size - 1;
        vector<int> markedEraseIndex;

        checkAndMergeWrites(startAddr, endAddr);
        markEraseBeforeWritesInBuffer(markedEraseIndex);
        checkAndMergeErase(startAddr, endAddr, markedEraseIndex);

        addEraseCmdToBuffer(startAddr, endAddr);

        return true;
    }

    bool checkAndReadFromBuffer(const commandParams& cmd) {
        struct commandParams bufferCommand;
        for (int i = buffer.getFilledCount(); i > 0; i--) {
            buffer.readAndParseBuffer(i, bufferCommand);
            if (bufferCommand.op == "W") {
                if (bufferCommand.addr == cmd.addr) {
                    file.updateOutput(bufferCommand.value);
                    return true;
                }
            }
            if (bufferCommand.op == "E") {
                for (int checkAddr = bufferCommand.addr; checkAddr < bufferCommand.addr + bufferCommand.size; checkAddr++) {
                    if (checkAddr == cmd.addr) {
                        file.updateOutput("0x00000000");
                        return true;
                    }
                }
            }
        }
        return false;
    }

    void CheckBufferCmdErasable(const commandParams& cmd) {
        for (int i = buffer.getFilledCount() - 1; i > 0; i--) {
            struct commandParams bufferCommand;
            buffer.readAndParseBuffer(i, bufferCommand);
            if (bufferCommand.op == "W") {
                if (bufferCommand.addr == cmd.addr) {
                    buffer.eraseBuffer(i);
                }
            }

            if (bufferCommand.op == "E") {
                if ((bufferCommand.size == 1) && (bufferCommand.addr == cmd.addr)) {
                    buffer.eraseBuffer(i);
                }
            }
        }
    }

    void addEraseCmdToBuffer(int startAddr, int endAddr) {
        struct commandParams ssdParams;
        ssdParams.op = "E";
        ssdParams.addr = startAddr;
        ssdParams.size = endAddr - startAddr + 1;

        buffer.writeBuffer(ssdParams);
        file.updateOutput("");
    }

    void checkAndMergeErase(int& startAddr, int& endAddr, std::vector<int>& markedEraseIndex) {
        struct commandParams bufferCommand;

        for (int i = buffer.getFilledCount(); i > 0; i--) {
            buffer.readAndParseBuffer(i, bufferCommand);

            if (bufferCommand.op == "E") {
                int bufStartAddr = bufferCommand.addr;
                int bufEndAddr = bufferCommand.addr + bufferCommand.size - 1;
                std::pair<int, int> merged = checkOverlap(startAddr, endAddr, bufStartAddr, bufEndAddr);

                if (merged.first == -1)
                    continue;
                if (merged.second - merged.first >= 10)
                    continue;
                if (find(markedEraseIndex.begin(), markedEraseIndex.end(), i) != markedEraseIndex.end())
                    continue;

                buffer.eraseBuffer(i);
                startAddr = merged.first;
                endAddr = merged.second;
            }
        }
    }

    void markEraseBeforeWritesInBuffer(std::vector<int>& markedEraseIndex) {
        struct commandParams bufferCommand;
        for (int i = 1; i <= buffer.getFilledCount(); i++) {
            buffer.readAndParseBuffer(i, bufferCommand);

            if (bufferCommand.op == "E") {
                for (int j = i + 1; j <= buffer.getFilledCount(); j++) {
                    struct commandParams innerBufferCommand;
                    buffer.readAndParseBuffer(j, innerBufferCommand);
                    if (innerBufferCommand.op == "W" &&
                        innerBufferCommand.addr >= bufferCommand.addr &&
                        innerBufferCommand.addr <= bufferCommand.addr + bufferCommand.size - 1) {
                        markedEraseIndex.push_back(i);
                    }
                }
            }
        }
    }

    void checkAndMergeWrites(int startAddr, int endAddr) {
        struct commandParams bufferCommand;
        for (int i = buffer.getFilledCount(); i > 0; i--) {
            buffer.readAndParseBuffer(i, bufferCommand);

            if (bufferCommand.op == "W") {
                if (bufferCommand.addr >= startAddr && bufferCommand.addr <= endAddr) {
                    buffer.eraseBuffer(i);
                }
            }
        }
    }

    void flushBufferandAddCurrentCmd(const commandParams& cmd) {
        if (flushBuffer() == false) {
            file.updateOutput("ERROR");
        }

        buffer.writeBuffer(cmd);
        file.updateOutput("");
    }

    std::pair<int, int> checkOverlap(int a, int b, int c, int d) {
        if (b + 1 >= c && a <= d + 1) {
            return { std::min(a, c), std::max(b, d) };
        }
        return { -1, -1 };
    }

    string buildCommand(struct commandParams& commandParam) {
        //string cmd, int lba, string data = ""
        string cmdLine = commandParam.op + " " + std::to_string(commandParam.addr);
        if (commandParam.op == "W") cmdLine = cmdLine + " " + commandParam.value;
        if (commandParam.op == "E") cmdLine = cmdLine + " " + std::to_string(commandParam.size);
        return cmdLine;
    }

    bool flushBuffer() {
        struct commandParams commandParam;
        bool bIsPass = false;
        const int buffer_head = 1;
        while (buffer.getFilledCount() != 0) {
            bIsPass = buffer.readAndParseBuffer(buffer_head, commandParam);
            if (bIsPass == false) return false;

            string cmdLine = buildCommand(commandParam);

            bIsPass = ssd->executeWithParam(commandParam);
            if (bIsPass == false) return false;      

            buffer.eraseBuffer(buffer_head);    
        }

        return true;
    }

    CommandParser parser;
    RealSSD* ssd; // RealSSD instance
    Buffer buffer; // Buffer instance to manage buffered operations
};

