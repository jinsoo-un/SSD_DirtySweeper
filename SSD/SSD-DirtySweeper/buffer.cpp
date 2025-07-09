#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <sstream>

using std::string;
using std::stringstream;
using std::vector;

namespace BufferFileNames {
    const string DIR_NAME = "./buffer";
    const string EMPTY_FILE_NAME = "_empty";
}
using namespace BufferFileNames;

struct params {
    string op;
    int addr;
    string value;
    int size;
};

class Buffer {
public:
    void initBuffer() {
        if (std::filesystem::exists(DIR_NAME) && std::filesystem::is_directory(DIR_NAME))
            return;

        std::filesystem::create_directory(DIR_NAME);

        for (int i = 1; i <= 5; ++i) {
            std::string bufferName = DIR_NAME + "/" + std::to_string(i) + EMPTY_FILE_NAME;
            std::ofstream ofs(bufferName);
            ofs.close();
        }
    }

    // Parse buffer which name is "[index]_*", return false if empty
    bool readAndParseBuffer(int index, struct params& output) {
        if (isOneBufferEmpty(index))
            return false;

        string data = readBuffer(index);

        vector<string> words;
        stringstream ss(data);
        string word;
        while (getline(ss, word, '_'))
            words.push_back(word);

        if (words.size() >= 4) {
            output.op = words[1];
            output.addr = stoi(words[2]);
            if (output.op == "W")
                output.value = words[3];
            else if (output.op == "E")
                output.size = stoi(words[3]);
            else
                return false;
        }
        return true;
    }

    bool isAllBufferEmpty() {
        for (int i = 1; i <= 5; i++) {
            if (!isOneBufferEmpty(i))
                return false;
        }
        return true;
    }

    // change the buffer "[index]_*" to "[index]_[data]"
    void writeBuffer(int index, struct params input) {
        string data = paramToBufferName(input);
        string targetBuffer = DIR_NAME + "/" + readBuffer(index);
        string newBuffer = DIR_NAME + "/" + std::to_string(index) + "_" + data;
        std::filesystem::rename(targetBuffer, newBuffer);
    }

private:
    string paramToBufferName(struct params input) {
        string result = "";
        if (input.op == "W")
            result = input.op + "_" + std::to_string(input.addr) + "_" + input.value;
        else if (input.op == "E")
            result = input.op + "_" + std::to_string(input.addr) + "_" + std::to_string(input.size);

        return result;
    }

    bool isOneBufferEmpty(int index) {
        string targetBuffer = readBuffer(index);
        string emptyBuffer = std::to_string(index) + EMPTY_FILE_NAME;
        if (targetBuffer != emptyBuffer)
            return false;
        return true;
    }

    // return the buffer "[index]_*"
    string readBuffer(int index) {
        if (index < 1 || index > 5)
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
};
