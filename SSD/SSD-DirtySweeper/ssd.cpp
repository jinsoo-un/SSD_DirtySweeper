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
        
        file.updateOutput("");
        buffer.writeBuffer(cmd);
        CheckBufferCmdErasable(cmd);
  
        return true;        
    }

    bool erase(const commandParams& cmd) {
        if (buffer.isFull()) {
            flushBufferandAddCurrentCmd(cmd);
            return true;
        }
  
        vector<int> markedEraseIndex;

        eraseAddressMatchedWriteCmd(cmd);
        markEraseBeforeWritesInBuffer(markedEraseIndex);
        checkAndMergeErase(cmd, markedEraseIndex);

        return true;
    }

    bool isAddressIncludedWriteCmd(commandParams& cmd, const int& startAddr, const int& endAddr) {
        if (cmd.op == "W") {
            if (cmd.addr >= startAddr && cmd.addr <= endAddr) {
                return true;
            }
        }
        return false;
    }

    bool isAddressMatchedWriteCmd(commandParams& cmd, const int& addr) {
        if (cmd.op == "W" && (cmd.addr == addr)) return true;
        return false;
    }

    bool isAddressMatchedEraseCmd(commandParams& cmd, const int& addr) {
        for (int checkAddr = cmd.addr; checkAddr < cmd.addr + cmd.size; checkAddr++) {
            if ((cmd.op == "E") && (checkAddr == addr)) return true;
        }
        return false;
    }

    bool checkAndReadFromBuffer(const commandParams& cmd) {
        struct commandParams bufferCommand;
        for (int i = buffer.getFilledCount(); i > 0; i--) {
            buffer.readAndParseBuffer(i, bufferCommand);
            
            if (isAddressMatchedWriteCmd(bufferCommand, cmd.addr)) {
                file.updateOutput(bufferCommand.value);
                return true;
            }

            if (isAddressMatchedEraseCmd(bufferCommand, cmd.addr)) {
                file.updateOutput("0x00000000");
                return true;
            }
        }
        return false;
    }

    void CheckBufferCmdErasable(const commandParams& cmd) {
        for (int i = buffer.getFilledCount() - 1; i > 0; i--) {
            struct commandParams bufferCommand;
            buffer.readAndParseBuffer(i, bufferCommand);
            
            if (isAddressMatchedWriteCmd(bufferCommand, cmd.addr)) {
                buffer.eraseBuffer(i);
            }

            if ((bufferCommand.size == 1) && isAddressMatchedEraseCmd(bufferCommand, cmd.addr)) {
                buffer.eraseBuffer(i);
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

    void checkAndMergeErase(const commandParams& cmd, std::vector<int>& markedEraseIndex) {
        struct commandParams bufferCommand;

        int startAddr = cmd.addr;
        int endAddr = startAddr + cmd.size - 1;

        for (int i = buffer.getFilledCount(); i > 0; i--) {
            buffer.readAndParseBuffer(i, bufferCommand);
            if (bufferCommand.op != "E") continue;

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

        addEraseCmdToBuffer(startAddr, endAddr);
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

    void eraseAddressMatchedWriteCmd(const commandParams& cmd) {
        int startAddr = cmd.addr;
        int endAddr = startAddr + cmd.size - 1;

        struct commandParams bufferCommand;
        for (int i = buffer.getFilledCount(); i > 0; i--) {
            buffer.readAndParseBuffer(i, bufferCommand);
            if(isAddressIncludedWriteCmd(bufferCommand, startAddr, endAddr))
                buffer.eraseBuffer(i);
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

    bool flushBuffer() {
        struct commandParams commandParam;
        bool bIsPass = false;
        const int buffer_head = 1;
        while (buffer.getFilledCount() != 0) {
            bIsPass = buffer.readAndParseBuffer(buffer_head, commandParam);
            if (bIsPass == false) return false;

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

