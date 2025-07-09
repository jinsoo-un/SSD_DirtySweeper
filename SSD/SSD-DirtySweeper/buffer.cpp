#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <sstream>

using std::string;
using std::stringstream;
using std::vector;

namespace BufferFileNames{
    const string DIR_NAME = "./buffer";
    const string EMPTY_FILE_NAME = "_empty";
}
using namespace BufferFileNames;

class Buffer {
public:
	void initBuffer() {
        if (std::filesystem::exists(DIR_NAME) && std::filesystem::is_directory(DIR_NAME))
            return;

        std::filesystem::create_directory(DIR_NAME); // 디렉토리 생성

        // 1_x, 2_x, ..., 5_x 파일 생성
        for (int i = 1; i <= 5; ++i) {
            std::string bufferName = DIR_NAME + "/" + std::to_string(i) + EMPTY_FILE_NAME;
            std::ofstream ofs(bufferName); // 빈 파일 생성
            ofs.close();
        }
	}

    // index의 buffer를 parsing해서 return, false if empty
    bool readAndParseBuffer(int index, string& op, int& addr, string &value, int& size) {
        if (isOneBufferEmpty(index))
            return false;

        string data = readBuffer(index);

        // 구분자를 기준으로 문자열 분할
        vector<string> words;
        stringstream ss(data);
        string word;
        while (getline(ss, word, '_'))
            words.push_back(word);

        if (words.size() >= 4) {
            op = words[1];
            addr = stoi(words[2]);
            if (op == "W")
                value = words[3];
            else if (op == "E")
                size = stoi(words[3]);
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

    // index의 buffer를 data로 변경
	void writeBuffer(int index, string data) {
        string targetBuffer = readBuffer(index);
        string newBuffer = DIR_NAME + "/" + std::to_string(index) + "_" + data;
        std::filesystem::rename(DIR_NAME + "/" + targetBuffer, newBuffer);
	}

private:
    bool isOneBufferEmpty(int index) {
        string targetBuffer = readBuffer(index);
        string emptyBuffer = std::to_string(index) + EMPTY_FILE_NAME;
        if (targetBuffer != emptyBuffer)
            return false;
        return true;
    }

    // index를 prefix로 가진 buffer를 return
    string readBuffer(int index) {
        if (index < 1 || index > 5)
            return nullptr;

        string prefix = std::to_string(index) + "_";
        for (const auto& entry : std::filesystem::directory_iterator(DIR_NAME)) {
            if (entry.is_regular_file()) {
                string filename = entry.path().filename().string();
                if (filename.rfind(prefix, 0) == 0) { // prefix로 시작하는지 확인
                    return filename;
                }
            }
        }

        return nullptr;
    }
};