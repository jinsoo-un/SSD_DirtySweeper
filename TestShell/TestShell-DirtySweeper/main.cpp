#include "gmock/gmock.h"
#include "testShell.cpp"

#ifdef NDEBUG
int main() {
    SsdHelpler ssd;
    TestShell shell(&ssd);

    std::string input;

    std::cout << "TestShell started. Type 'help' for commands.\n";

    while (true) {
        std::cout << "Shell> ";
        std::getline(std::cin, input);

        if (input == "exit") {
            std::cout << "Exiting...\n";
            break;
        }

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