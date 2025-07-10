#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <sstream>

#include "commandParam.h"

using std::string;
using std::stringstream;
using std::vector;

const int MAX_BUFFER_SIZE = 5;

namespace BufferFileNames {
    const string DIR_NAME = "./buffer";
    const string EMPTY_FILE_NAME = "_empty";
}
using namespace BufferFileNames;

class Buffer {
public:
    Buffer() {
        initBuffer();
	}

    bool readAndParseBuffer(int index, struct commandParams& output) {
        if (isOneBufferEmpty(index))
            return false;

        string data = readBuffer(index);
        vector<string> words;
        stringstream ss(data);
        string word;

        while (getline(ss, word, '_'))
            words.push_back(word);

        if (parseBufferToParam(words, output))
            return true;
        
        return false;
    }

    int getFilledCount() {
        for (int i = 1; i <= MAX_BUFFER_SIZE; i++) {
            if (isOneBufferEmpty(i))
                return i - 1;
        }
        return MAX_BUFFER_SIZE;
    }

    void eraseBuffer(int index) {
        vector<string> suffixes;
        string targetBuffer = DIR_NAME + "/" + readBuffer(index);
        string newBuffer = getEmptyBufferName(index);

        std::filesystem::rename(targetBuffer, newBuffer);

        storeBufferSuffixesAfterIndex(index, suffixes);
        renameBuffersAfterIndex(index, suffixes);
    }

    void writeBuffer(struct commandParams input) {
        string data = paramToBufferName(input);
        int index = getFilledCount();
        if (index >= MAX_BUFFER_SIZE)
            return;

        string targetBuffer = DIR_NAME + "/" + readBuffer(index + 1);
        string newBuffer = DIR_NAME + "/" + std::to_string(index + 1) + "_" + data;
        std::filesystem::rename(targetBuffer, newBuffer);
    }

    bool isEmpty() {
        for (int i = 1; i <= MAX_BUFFER_SIZE; i++) {
            if (!isOneBufferEmpty(i))
                return false;
        }
        return true;
    }

    bool isFull() {
        for (int i = 1; i <= MAX_BUFFER_SIZE; i++) {
            if (isOneBufferEmpty(i))
                return false;
        }
        return true;
    }

    void clear() {
        for (int i = 1; i <= MAX_BUFFER_SIZE; i++) {
            string targetBuffer = DIR_NAME + "/" + readBuffer(i);
            string newBuffer = getEmptyBufferName(i);
            std::filesystem::rename(targetBuffer, newBuffer);
        }
    }

private:
    void initBuffer() {
        if (std::filesystem::exists(DIR_NAME) && std::filesystem::is_directory(DIR_NAME))
            return;

        std::filesystem::create_directory(DIR_NAME);

        for (int i = 1; i <= MAX_BUFFER_SIZE; ++i) {
            std::string bufferName = getEmptyBufferName(i);
            std::ofstream ofs(bufferName);
            ofs.close();
        }
    }

    bool parseBufferToParam(vector<string>& words, commandParams& output) {
        if (words.size() >= 4) {
            output.op = words[1];
            output.addr = stoi(words[2]);
            if (output.op == "W") {
                output.value = words[3];
                output.size = 0;
            }
            else if (output.op == "E") {
                output.value = "0x0";
                output.size = stoi(words[3]);
            }
            else
                return false;
        }
        return true;
    }

    string getEmptyBufferName(int index) {
        return DIR_NAME + "/" + std::to_string(index) + EMPTY_FILE_NAME;
    }

    string paramToBufferName(struct commandParams input) {
        string result = "";
        if (input.op == "W")
            result = input.op + "_" + std::to_string(input.addr) + "_" + input.value;
        else if (input.op == "E")
            result = input.op + "_" + std::to_string(input.addr) + "_" + std::to_string(input.size);

        return result;
    }

    // return the buffer "[index]_*"
    string readBuffer(int index) {
        if (index < 1 || index > MAX_BUFFER_SIZE)
            return nullptr;

        string prefix = std::to_string(index) + "_";
        for (const auto& entry : std::filesystem::directory_iterator(DIR_NAME)) {
            if (entry.is_regular_file()) {
                string filename = entry.path().filename().string();
                if (filename.rfind(prefix, 0) == 0) {
                    return filename;
                }
            }
        }

        return nullptr;
    }

    bool isOneBufferEmpty(int index) {
        string targetBuffer = readBuffer(index);
        string emptyBuffer = std::to_string(index) + EMPTY_FILE_NAME;
        if (targetBuffer != emptyBuffer)
            return false;
        return true;
    }

    void storeBufferSuffixesAfterIndex(int index, std::vector<std::string>& suffixes) {
        for (int i = index + 1; i <= MAX_BUFFER_SIZE; i++) {
            string filename = readBuffer(i);
            size_t pos = filename.find('_');
            string result;
            if (pos != string::npos) {
                result = filename.substr(pos + 1);
            }
            suffixes.push_back(result);
        }
    }

    void renameBuffersAfterIndex(int index, std::vector<std::string>& suffixes) {
        string targetBuffer;
        string newBuffer;

        for (int i = index; i <= MAX_BUFFER_SIZE - 1; i++) {
            targetBuffer = DIR_NAME + "/" + readBuffer(i);
            newBuffer = DIR_NAME + "/" + std::to_string(i) + "_" + suffixes[i - index];
            std::filesystem::rename(targetBuffer, newBuffer);
        }

        targetBuffer = DIR_NAME + "/" + readBuffer(MAX_BUFFER_SIZE);
        newBuffer = getEmptyBufferName(MAX_BUFFER_SIZE);
        std::filesystem::rename(targetBuffer, newBuffer);
    }
};