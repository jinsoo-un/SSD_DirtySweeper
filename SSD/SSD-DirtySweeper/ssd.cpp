#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include "command.cpp"
#include "file.h"
#include "buffer.cpp"

using namespace std;

using std::string;

const int VALID_DATA_LENGTH = 10;

// SSD Interface Class
class SSD {
public:
	virtual bool parseCommand(string command) = 0;
	virtual bool exec() = 0;
	virtual int getArgCount() = 0;
	virtual string getOp() = 0;
	virtual int getAddr() = 0;
	virtual string getValue() = 0;
	virtual int getSize() = 0;
    virtual int getAccessCount() = 0;
    virtual void bufferClear() = 0;
protected:
	FileControl& file = FileControl::get_instance();
};

class RealSSD : public SSD {
public:
	bool parseCommand(string command) {
        if (!isValidCommand(command)) {
	        file.updateOutput("ERROR");
	        return false;
        }
        storeParams(command);
        return true;
	}

	bool exec() {
		ReadCommand readCmd;
		WriteCommand writeCmd;
		EraseCommand eraseCmd;

		if (op == "R")
			setCommand(&readCmd);
        if (op == "W") {
            setCommand(&writeCmd);
            accessCount++;
        }
        if (op == "E") {
            setCommand(&eraseCmd);
            accessCount += size;
        }

		if (command == nullptr)
			return false;

		return command->run(addr, value, size);
	}

	int getArgCount() {
		return argCount;
	}

	string getOp() {
		return op;
	}

	int getAddr() {
		return addr;
	}

	string getValue() {
		return value;
	}

	int getSize() {
		return size;
	}

    int getAccessCount() {
        return accessCount;
    }

    void bufferClear() {
        return;
    }
private:
	void storeParams(string command)
	{
        std::istringstream iss(command);
        string arg;
        int cnt = 0;
        /* scan the command line input */
        while (iss >> arg) {
	        cnt++;
	        if (cnt == 1)
		        op = arg;
	        if (cnt == 2)
		        addr = std::stoi(arg);
			if (cnt == 3) {
				if (op == "E") size = std::stoi(arg);
				else value = arg;
			}
        }
        argCount = cnt;
	}

	bool isAddressOutOfRange(int address) {
		return address < MIN_ADDRESS || address >= MAX_ADDRESS;
	}
	
	bool isEraseOutOfRange(int address, int size) {
		if (address + size > MAX_ADDRESS)
			return true;
		if (size < 1 || size > 10)
			return true;
		return false;
	}

	bool isValidCommand(string command) {
        std::istringstream iss(command);
        string arg;
        int cnt = 0;
		int tmpAddr = 0;
		bool isErase = false;

        while (iss >> arg) {
	        cnt++;
	        if (cnt == 1) {
		        if (!isValidOp(arg)) return false;
				if (arg == "E") isErase = true;
	        }
	        else if (cnt == 2) {
		        if (isAddressOutOfRange(stoi(arg))) return false;
				if (isErase) tmpAddr = stoi(arg);
	        }
	        else if (cnt == 3) {
				if (isErase) {
					if (isEraseOutOfRange(tmpAddr, stoi(arg)))
						return false;
					else
						continue;
				}
		        if (!isValidWriteData(arg)) return false;
	        }
	        else
		        return false;
        }

        return true;
	}

	bool isValidOp(string arg) {
        if (arg != "R" && arg != "W" && arg != "E")
	        return false;
        return true;
	}

	bool isValidWriteData(const std::string& str) {
		if (str.substr(0, 2) != "0x") return false;

		int length;
		for (length = 2; length < str.size(); length++) {
			char ch = static_cast<unsigned char>(str[length]);

			if (!(isNumber(ch) || isHexCharacter(ch))) { return false; }

		}

		if (length != VALID_DATA_LENGTH) { return false; }

		return true;
	}

	bool isHexCharacter(char ch) {
		return ((ch >= 'A') && (ch <= 'F'));
	}

	bool isNumber(char ch) {
		return ((ch >= '0') && (ch <= '9'));
	}

	void setCommand(SSDCommand* cmd) {
		command = cmd;
	}

	int argCount;
	string op;
	int addr;
	string value;
	int size;
    int accessCount;

	SSDCommand* command = nullptr;
};

// SSD Proxy Class
class BufferedSSD : public SSD {
public:
	BufferedSSD() : ssd{ new RealSSD() } {}
	bool parseCommand(string command) {
		return ssd->parseCommand(command);
	}
	bool exec() {
		string operation = ssd->getOp();
		if (operation == "R") return read();
		if (operation == "W") return write();
		if (operation == "E") return erase();	
	}
	int getArgCount() {
		return ssd->getArgCount();
	}
	string getOp() {
		return ssd->getOp();
	}
	int getAddr() {
		return ssd->getAddr();
	}
	string getValue() {
		return ssd->getValue();
	}
	int getSize() {
		return ssd->getSize();
	}

    int getAccessCount() {
        return ssd->getAccessCount();
    }

    void bufferClear() {
        buffer.clear();
        return;
    }
private:
	// Buffered SSD methods
	bool read() {
		// Buffer 에 아무것도 없는 경우 RealSSD 에서 읽어오기
		if (buffer.isEmpty()) return ssd->exec();
		// Buffer 에 있는 마지막 명령어부터 Check
		for (int i = buffer.getFilledCount(); i > 0; i--) {
			struct params bufferCommand;
			buffer.readAndParseBuffer(i, bufferCommand);
			// buffer command 가 write 인 경우
			if (bufferCommand.op == "W") {
				// Address가 일치하는 경우
				if (bufferCommand.addr == ssd->getAddr()) {
					// Return the value from buffer
					file.updateOutput(bufferCommand.value);
					return true;
				}
			}
			// buffer command 가 erase 인 경우
			if (bufferCommand.op == "E") {
				// Address가 일치하는 경우
				for (int checkAddr = bufferCommand.addr; checkAddr < bufferCommand.addr + bufferCommand.size; checkAddr++) {
					if (checkAddr == ssd->getAddr()) {
						// Return the initial value
						file.updateOutput("0x00000000");
						return true;
					}
				}
			}
		}

		// if all command buffer is not matched, read from RealSSD
		return ssd->exec(); // read from RealSSD
	}

	bool write() {
        struct params ssdParams;
        ssdParams.op = ssd->getOp();
        ssdParams.addr = ssd->getAddr();
        ssdParams.value = ssd->getValue();

        if (buffer.isFull()) {
            flushBuffer();
            buffer.clear();
        }

        buffer.writeBuffer(ssdParams);

        if (buffer.getFilledCount() == 1) return true;

        for (int i = buffer.getFilledCount()-1; i > 0; i--) {
            struct params bufferCommand;
            buffer.readAndParseBuffer(i, bufferCommand);
            if (bufferCommand.op == "W") {
                if (bufferCommand.addr == ssd->getAddr()) {
                     buffer.eraseBuffer(i);
                    return true;
                }
            }

            if (bufferCommand.op == "E") {
                if ((bufferCommand.size == 1) && (bufferCommand.addr == ssd->getAddr())) {
                    buffer.eraseBuffer(i);
                    return true;
                }
            }
        }
  
        return false;        
	}

	bool erase() {
		// check if buffer is full, flush to RealSSD
		if (buffer.isFull()) {
			struct params ssdParams;
			ssdParams.op = ssd->getOp();
			ssdParams.addr = ssd->getAddr();
			ssdParams.size = ssd->getSize();
			flushBuffer();
			// write the command to buffer
			buffer.writeBuffer(ssdParams);
			file.updateOutput("");
			return true;
		}

		int startAddr = ssd->getAddr();
		int endAddr = startAddr + ssd->getSize() - 1;

		// check ascending writes
		int index = buffer.getFilledCount();
		for (int i = index; i > 0; i--) {
			struct params bufferCommand;
			buffer.readAndParseBuffer(i, bufferCommand);

			// check if writes are in erase range
			if (bufferCommand.op == "W") {
				if (bufferCommand.addr >= startAddr && bufferCommand.addr <= endAddr) {
					buffer.eraseBuffer(i);
				}
			}
		}

		index = buffer.getFilledCount();
		vector<int> markedEraseIndex;

		for (int i = 1; i <= index; i++) {
			struct params bufferCommand;
			buffer.readAndParseBuffer(i, bufferCommand);

			// write after erase, just mark that erase if matters
			if (bufferCommand.op == "E") {
				for (int j = i + 1; j <= index; j++) {
					struct params innerBufferCommand;
					buffer.readAndParseBuffer(j, innerBufferCommand);
					if (innerBufferCommand.op == "W" &&
						innerBufferCommand.addr >= bufferCommand.addr &&
						innerBufferCommand.addr <= bufferCommand.addr + bufferCommand.size - 1) {
						markedEraseIndex.push_back(i);
					}
				}
			}
		}

		// check ascending erases, merge if it can
		index = buffer.getFilledCount();
		for (int i = index; i > 0; i--) {
			struct params bufferCommand;
			buffer.readAndParseBuffer(i, bufferCommand);
			
			// check if ascending erase can be merged
			if (bufferCommand.op == "E") {
				int bufStartAddr = bufferCommand.addr;
				int bufEndAddr = bufferCommand.addr + bufferCommand.size - 1;
				std::pair<int, int> merged = checkOverlap(startAddr, endAddr, bufStartAddr, bufEndAddr);
				
				// if no overlapped section, do nothing
				if (merged.first == -1)
					continue;
				// if overlapped section's size exceed 10, do nothing
				if (merged.second - merged.first >= 10)
					continue;
				// if target erase command is marked before, do nothing
				if (find(markedEraseIndex.begin(), markedEraseIndex.end(), i) != markedEraseIndex.end())
					continue;

				buffer.eraseBuffer(i);
				startAddr = merged.first;
				endAddr = merged.second;
			}
		}

		struct params ssdParams;
		ssdParams.op = ssd->getOp();
		ssdParams.addr = startAddr;
		ssdParams.size = endAddr - startAddr + 1;

		// write the command to buffer
		buffer.writeBuffer(ssdParams);
		file.updateOutput("");

		return true;
	}

	std::pair<int, int> checkOverlap(int a, int b, int c, int d) {
		if (b >= c && a <= d) {
			return { std::min(a, c), std::max(b, d) };
		}
		return { -1, -1 };
	}

	string buildCommand(struct params& commandParam) {
		//string cmd, int lba, string data = ""
		string cmdLine = commandParam.op + " " + std::to_string(commandParam.addr);
		if (commandParam.op == "W") cmdLine = cmdLine + " " + commandParam.value;
		if (commandParam.op == "E") cmdLine = cmdLine + " " + std::to_string(commandParam.size);
		return cmdLine;
	}

	void flushBuffer() {
		// Flush the buffer to RealSSD
		for (int i = 1; i <= buffer.getFilledCount(); i++) {
			struct params commandParam;
			if (buffer.readAndParseBuffer(i, commandParam)) {
				string cmdLine = buildCommand(commandParam);
				ssd->parseCommand(cmdLine);
				ssd->exec();
			}
		}
		buffer.clear();
	}

	RealSSD* ssd; // RealSSD instance
	Buffer buffer; // Buffer instance to manage buffered operations
	
};

