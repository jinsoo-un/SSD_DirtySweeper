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
		command = factory.getCommand(op);
		if (command == nullptr) return false;

		UpdateAccessCount(op);

		return command->run(addr, value, size);
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
        if (arg != "R" && arg != "W" && arg != "E" && arg != "F")
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

	void UpdateAccessCount(const string& cmd)
	{
		if (cmd == "W") accessCount++;
		if (cmd == "E") accessCount += size;
	}

	string op;
	int addr;
	string value;
	int size;
    int accessCount;

	SSDCommand* command = nullptr;
	SSDCommandFactory factory;
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
		if (buffer.isEmpty())
			return ssd->exec();

		if (checkAndReadFromBuffer())
				return true;

		return ssd->exec();
	}

	bool write() {
        struct params ssdParams;
        ssdParams.op = ssd->getOp();
        ssdParams.addr = ssd->getAddr();
        ssdParams.value = ssd->getValue();

        if (buffer.isFull()) {
            if (flushBuffer() == false) {
                file.updateOutput("ERROR");
            }
        }

        buffer.writeBuffer(ssdParams);
		checkAndEraseBuffer();
  
        return true;        
	}

	bool erase() {
		if (buffer.isFull()) {
			flushBufferandAddCurrentCmd();
			return true;
		}

		int startAddr = ssd->getAddr();
		int endAddr = startAddr + ssd->getSize() - 1;
		vector<int> markedEraseIndex;

		checkAndMergeWrites(startAddr, endAddr);
		markEraseBeforeWritesInBuffer(markedEraseIndex);
		checkAndMergeErase(startAddr, endAddr, markedEraseIndex);

		addEraseCmdToBuffer(startAddr, endAddr);

		return true;
	}

	bool checkAndReadFromBuffer() {
		struct params bufferCommand;
		for (int i = buffer.getFilledCount(); i > 0; i--) {
			buffer.readAndParseBuffer(i, bufferCommand);
			if (bufferCommand.op == "W") {
				if (bufferCommand.addr == ssd->getAddr()) {
					file.updateOutput(bufferCommand.value);
					return true;
				}
			}
			if (bufferCommand.op == "E") {
				for (int checkAddr = bufferCommand.addr; checkAddr < bufferCommand.addr + bufferCommand.size; checkAddr++) {
					if (checkAddr == ssd->getAddr()) {
						file.updateOutput("0x00000000");
						return true;
					}
				}
			}
		}
		return false;
	}

	void checkAndEraseBuffer() {
		for (int i = buffer.getFilledCount() - 1; i > 0; i--) {
			struct params bufferCommand;
			buffer.readAndParseBuffer(i, bufferCommand);
			if (bufferCommand.op == "W") {
				if (bufferCommand.addr == ssd->getAddr()) {
					buffer.eraseBuffer(i);
				}
			}

			if (bufferCommand.op == "E") {
				if ((bufferCommand.size == 1) && (bufferCommand.addr == ssd->getAddr())) {
					buffer.eraseBuffer(i);
				}
			}
		}
	}

	void addEraseCmdToBuffer(int startAddr, int endAddr) {
		struct params ssdParams;
		ssdParams.op = ssd->getOp();
		ssdParams.addr = startAddr;
		ssdParams.size = endAddr - startAddr + 1;

		buffer.writeBuffer(ssdParams);
		file.updateOutput("");
	}

	void checkAndMergeErase(int& startAddr, int& endAddr, std::vector<int>& markedEraseIndex) {
		struct params bufferCommand;

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
		struct params bufferCommand;
		for (int i = 1; i <= buffer.getFilledCount(); i++) {
			buffer.readAndParseBuffer(i, bufferCommand);

			if (bufferCommand.op == "E") {
				for (int j = i + 1; j <= buffer.getFilledCount(); j++) {
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
	}

	void checkAndMergeWrites(int startAddr, int endAddr) {
		struct params bufferCommand;
		for (int i = buffer.getFilledCount(); i > 0; i--) {
			buffer.readAndParseBuffer(i, bufferCommand);

			if (bufferCommand.op == "W") {
				if (bufferCommand.addr >= startAddr && bufferCommand.addr <= endAddr) {
					buffer.eraseBuffer(i);
				}
			}
		}
	}

	void flushBufferandAddCurrentCmd() {
		struct params ssdParams;
		ssdParams.op = ssd->getOp();
		ssdParams.addr = ssd->getAddr();
		ssdParams.size = ssd->getSize();

		if (flushBuffer() == false) {
			file.updateOutput("ERROR");
		}

		buffer.writeBuffer(ssdParams);
		file.updateOutput("");
	}

	std::pair<int, int> checkOverlap(int a, int b, int c, int d) {
		if (b + 1 >= c && a <= d + 1) {
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

	bool flushBuffer() {
        struct params commandParam;
        bool bIsPass = false;
        const int buffer_head = 1;
        while (buffer.getFilledCount() != 0) {
            bIsPass = buffer.readAndParseBuffer(buffer_head, commandParam);
            if (bIsPass == false) return false;

            string cmdLine = buildCommand(commandParam);
            ssd->parseCommand(cmdLine);

            bIsPass = ssd->exec();
            if (bIsPass == false) return false;      

            buffer.eraseBuffer(buffer_head);    
        }

        return true;
	}

	RealSSD* ssd; // RealSSD instance
	Buffer buffer; // Buffer instance to manage buffered operations
	
};

