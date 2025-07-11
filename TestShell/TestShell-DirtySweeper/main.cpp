#include "gmock/gmock.h"
#include "logger.h"
#include "ssd.h"
#include "testShell.h"

#ifdef NDEBUG
int main(int argc, char* argv[]) {
    SSDDriver ssd;
    TestShell shell(&ssd);

    if (argc == 2) {
        ifstream scriptFile(argv[1]);
        if (!scriptFile.is_open()) {
            cerr << "Failed to open script file: " << argv[1] << "\n";
            return 1;
        }

        string line;
        while (getline(scriptFile, line)) {
            if (line.empty()) continue;

            // 왼쪽 정렬 + 25칸 고정 너비로 명령어 출력
            cout << left << setw(27) << line << "___  Run..." << flush;

            try {
                stringstream buffer;
                streambuf* prevCoutBuf = cout.rdbuf(buffer.rdbuf()); // cout 캡처

                shell.processInput(line);

                cout.rdbuf(prevCoutBuf); // cout 복원
                string output = buffer.str();

                if (output.find("FAIL") != string::npos) {
                    cout << "FAIL!" << endl;
                    return 1;
                }
                else {
                    cout << "Pass" << endl;
                }

            }
            catch (...) {
                cout << "FAIL!" << endl;
                return 1;
            }
        }
        return 0;
    }
    else if (argc > 2) {
        cerr << "Usage: " << argv[0] << " [script_file]\n";
        return 0;
    }

    while (!shell.isExit()) {
        string input;
        cout << "Shell> ";
        getline(cin, input);

        try {
            shell.processInput(input);
        }
        catch (const exception& e) {
            cerr << "Error: " << e.what() << "\n";
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
