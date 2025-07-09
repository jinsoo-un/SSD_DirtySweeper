#include "gmock/gmock.h"
#include "testShell.cpp"

#ifdef NDEBUG
int main(int argc, char* argv[]) {
    SsdHelpler ssd;
    TestShell shell(&ssd);

    if (argc == 2) {
        std::ifstream scriptFile(argv[1]);
        if (!scriptFile.is_open()) {
            std::cerr << "Failed to open script file: " << argv[1] << "\n";
            return 1;
        }

        std::string line;
        while (std::getline(scriptFile, line)) {
            if (line.empty()) continue;

            // 왼쪽 정렬 + 25칸 고정 너비로 명령어 출력
            std::cout << std::left << std::setw(27) << line << "___  Run..." << std::flush;

            try {
                std::stringstream buffer;
                std::streambuf* prevCoutBuf = std::cout.rdbuf(buffer.rdbuf()); // cout 캡처

                shell.processInput(line);

                std::cout.rdbuf(prevCoutBuf); // cout 복원
                std::string output = buffer.str();

                if (output.find("FAIL") != std::string::npos) {
                    std::cout << "FAIL!" << std::endl;
                    return 1;
                }
                else {
                    std::cout << "Pass" << std::endl;
                }

            }
            catch (...) {
                std::cout << "FAIL!" << std::endl;
                return 1;
            }
        }
        return 0;
    }
    else if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [script_file]\n";
        return 0;
    }

    while (!shell.isExit()) {
        std::string input;
        std::cout << "Shell> ";
        std::getline(std::cin, input);

        try {
            shell.processInput(input);
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }

    return 0;
}

#else
int main() {
    ::testing::InitGoogleMock();
    return RUN_ALL_TESTS();
}
#endif