#include <iostream>
#include "ssd.cpp"
#include "gmock/gmock.h"
#include <thread>
#include <chrono>

using std::string;

class RealSSDTest : public ::testing::Test {
public:
    SSD* ssd = new RealSSD();
    string VALID_HEX_DATA = "0x1298CDEF";
    string INVALID_HEX_DATA = "0xABCDEFGH";
    string INITIAL_HEX_DATA = "0x00000000";
    static const int VALID_TEST_ADDRESS = 0;
    static const int VALID_TEST_ADDRESS_MAX = 99;
    static const int INVALID_TEST_ADDRESS = 100;
    static const int VALID_TEST_SIZE = 10;
    static const int INVALID_TEST_SIZE = 20;

	static const int DELAY_NANOS_FOR_WRITE = 1000000; // 1 ms

    void SetUp() override {
        ofstream file(FileNames::DATA_FILE);
        if (!file.is_open()) {
            cout << "Error opening file for setup test." << endl;
            return;
        }

        for (int i = MIN_ADDRESS; i < MAX_ADDRESS; i++) {
            file << i << "\t" << "0x00000000" << endl;
        }
        file.close();

        this_thread::sleep_for(chrono::nanoseconds(DELAY_NANOS_FOR_WRITE));
    }

    bool checkOutputFile(string expected) {
        ifstream fin(FileNames::OUTPUT_FILE);
        if (!fin.is_open()) {
            cout << "OUTPUT file open failed\n";
            return false;
        }

        string line;
        getline(fin, line);
        if (line != expected)
            return false;
        return true;
    }

    string buildCommand(string cmd, int lba, string data = "") {
        string cmdLine = cmd + " " + std::to_string(lba);
        if (cmd == "W" || cmd == "E") cmdLine = cmdLine + " " + data;
        return cmdLine;
    }

    bool parseAndExecute(const string& cmd)
    {
        bool isPass = ssd->parseCommand(cmd);
        if (isPass) {
            isPass = ssd->exec();
        }
        return isPass;
    }
};

TEST_F(RealSSDTest, ReadTC_InitialValue)
{
    string cmd = buildCommand("R", 1);
    bool isPass = parseAndExecute(cmd);

    EXPECT_EQ(true, isPass);
	EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(RealSSDTest, ReadTC_OutofRange)
{
    string cmd = buildCommand("R", 100);
    bool isPass = parseAndExecute(cmd);
    EXPECT_EQ(false, isPass);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(RealSSDTest, ReadTC_ReturnData01)
{
    string cmd = buildCommand("R", 50);
    bool isPass = parseAndExecute(cmd);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(RealSSDTest, ReadTC_ReturnData02)
{
    string cmd = buildCommand("R", 30);
    bool isPass = parseAndExecute(cmd);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(RealSSDTest, ArgparseRead) {
    string cmd = buildCommand("R", 3);
    ssd->parseCommand(cmd);
    EXPECT_EQ(2, ssd->getArgCount());
    EXPECT_EQ("R", ssd->getOp());
    EXPECT_EQ(3, ssd->getAddr());
}

TEST_F(RealSSDTest, ArgparseWrite)
{
    string cmd = buildCommand("W", 3, VALID_HEX_DATA);
    ssd->parseCommand(cmd);
    EXPECT_EQ(3, ssd->getArgCount());
    EXPECT_EQ("W", ssd->getOp());
    EXPECT_EQ(3, ssd->getAddr());
    EXPECT_EQ("0x1298CDEF", ssd->getValue());
}

TEST_F(RealSSDTest, ArgparseErase)
{
    string cmd = buildCommand("E", 3, std::to_string(10));
    ssd->parseCommand(cmd);
    EXPECT_EQ(3, ssd->getArgCount());
    EXPECT_EQ("E", ssd->getOp());
    EXPECT_EQ(3, ssd->getAddr());
	EXPECT_EQ(10, ssd->getSize());
}

TEST_F(RealSSDTest, ArgparseInvalidOp)
{
    string cmd = buildCommand("S", 3);
    ssd->parseCommand(cmd);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(RealSSDTest, ArgparseInvalidAddr)
{
    string cmd = buildCommand("R", 300);
    ssd->parseCommand(cmd);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(RealSSDTest, ArgparseInvalidValue)
{
    string cmd = buildCommand("W", 3, INVALID_HEX_DATA);
    ssd->parseCommand(cmd);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(RealSSDTest, WritePass) {
    string cmd = buildCommand("W", VALID_TEST_ADDRESS, VALID_HEX_DATA);
    ssd->parseCommand(cmd);
	bool isPass = ssd->exec();
    EXPECT_TRUE(isPass);
}

TEST_F(RealSSDTest, WriteFailWithOutOfAddressRange) {
    string cmd = buildCommand("W", INVALID_TEST_ADDRESS, VALID_HEX_DATA);
    bool isPass = parseAndExecute(cmd);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteInvalidData00) {
    string cmd = buildCommand("W", VALID_TEST_ADDRESS, "0x1234567890000");
    bool isPass = parseAndExecute(cmd);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteInvalidData01) {
    string cmd = buildCommand("W", VALID_TEST_ADDRESS, "0x1234");
    bool isPass = parseAndExecute(cmd);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteInvalidData02) {
    string cmd = buildCommand("W", VALID_TEST_ADDRESS, "12345678");
    bool isPass = parseAndExecute(cmd);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteInvalidData03) {
    string cmd = buildCommand("W", VALID_TEST_ADDRESS, "0x1234ABzE");
    bool isPass = parseAndExecute(cmd);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteInvalidData04) {
    string cmd = buildCommand("W", VALID_TEST_ADDRESS, "0xA5CCH012");
    bool isPass = parseAndExecute(cmd);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteReadVerify00) {
    string cmd = buildCommand("R", VALID_TEST_ADDRESS);
    bool isPass = parseAndExecute(cmd);

    this_thread::sleep_for(chrono::nanoseconds(DELAY_NANOS_FOR_WRITE));

    ASSERT_EQ(true, isPass);
    ASSERT_TRUE(checkOutputFile(INITIAL_HEX_DATA));

    cmd = buildCommand("W", VALID_TEST_ADDRESS, VALID_HEX_DATA);
    isPass = parseAndExecute(cmd);
    ASSERT_TRUE(isPass);

    this_thread::sleep_for(chrono::nanoseconds(DELAY_NANOS_FOR_WRITE));

    cmd = buildCommand("R", VALID_TEST_ADDRESS);
    isPass = parseAndExecute(cmd);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(VALID_HEX_DATA));
}

TEST_F(RealSSDTest, ErasePass) {
    string cmd = buildCommand("E", VALID_TEST_ADDRESS, to_string(VALID_TEST_SIZE));
    bool isPass = parseAndExecute(cmd);
    EXPECT_TRUE(isPass);
}

TEST_F(RealSSDTest, EraseFailOutofRange) {
    string cmd = buildCommand("E", INVALID_TEST_ADDRESS, to_string(VALID_TEST_SIZE));
    bool isPass = parseAndExecute(cmd);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, EraseFailOutofRangeDestination) {
    string cmd = buildCommand("E", VALID_TEST_ADDRESS_MAX, to_string(VALID_TEST_SIZE));
    bool isPass = parseAndExecute(cmd);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, EraseFailExceedMaxSize) {
    string cmd = buildCommand("E", VALID_TEST_ADDRESS, to_string(INVALID_TEST_SIZE));
    bool isPass = parseAndExecute(cmd);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, EraseAndReadVerify) {
    string cmd = buildCommand("W", VALID_TEST_ADDRESS, VALID_HEX_DATA);
    bool isPass = parseAndExecute(cmd);
    ASSERT_TRUE(isPass);

    this_thread::sleep_for(chrono::nanoseconds(DELAY_NANOS_FOR_WRITE));
    
    cmd = buildCommand("E", VALID_TEST_ADDRESS, to_string(VALID_TEST_SIZE));
    isPass = parseAndExecute(cmd);
    ASSERT_TRUE(isPass);

    this_thread::sleep_for(chrono::nanoseconds(DELAY_NANOS_FOR_WRITE));

	cmd = buildCommand("R", VALID_TEST_ADDRESS);
    isPass = parseAndExecute(cmd);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

///////////////////////////////////////////////////////////////////////////
class BufSSDTest : public ::testing::Test {
public:
    SSD* ssd = new BufferedSSD();

    string VALID_HEX_DATA = "0x1298CDEF";
    string PRECONDITION_HEX_DATA = "0xABCDA5A5";
    string INITIAL_HEX_DATA = "0x00000000";

    static const int VALID_TEST_ADDRESS = 0;
    static const int VALID_TEST_ADDRESS_MAX = 99;
    static const int INVALID_TEST_ADDRESS = 100;
    static const int VALID_TEST_SIZE = 10;
    static const int INVALID_TEST_SIZE = 20;

	static const int DELAY_NANOS_FOR_WRITE = 1000000; // 1 ms
    
    string cmd;
    int lba;
    int lba_size;

    void SetUp() override {
        ofstream file(FileNames::DATA_FILE);
        if (!file.is_open()) {
            cout << "Error opening file for setup test." << endl;
            return;
        }

        for (int i = MIN_ADDRESS; i < MAX_ADDRESS; i++) {
            file << i << "\t" << "0x00000000" << endl;
        }
        file.close();

        ssd->bufferClear();


        this_thread::sleep_for(chrono::nanoseconds(DELAY_NANOS_FOR_WRITE));
    }

    bool checkOutputFile(string expected) {
        ifstream fin(FileNames::OUTPUT_FILE);
        if (!fin.is_open()) {
            cout << "OUTPUT file open failed\n";
            return false;
        }

        string line;
        getline(fin, line);
        if (line != expected)
            return false;
        return true;
    }

    string buildCommand(string cmd, int lba, string data = "") {
        string cmdLine = cmd + " " + std::to_string(lba);
        if (cmd == "W" || cmd == "E") cmdLine = cmdLine + " " + data;
        return cmdLine;
    }

    bool parseAndExecute(const string& cmd)
    {
        bool isPass = ssd->parseCommand(cmd);
        if (isPass) {
            isPass = ssd->exec();
        }
        return isPass;
    }

};

TEST_F(BufSSDTest, ReadTC_InitialValue)
{
    string cmd = buildCommand("R", 1);
    bool isPass = parseAndExecute(cmd);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(BufSSDTest, ReadTC_OutofRange)
{
    string cmd = buildCommand("R", 100);
    bool isPass = parseAndExecute(cmd);
    EXPECT_EQ(false, isPass);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(BufSSDTest, ReadTC_ReturnData01)
{
    string cmd = buildCommand("R", 50);
    bool isPass = parseAndExecute(cmd);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(BufSSDTest, ReadTC_ReturnData02)
{
    string cmd = buildCommand("R", 30);
    bool isPass = parseAndExecute(cmd);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(BufSSDTest, SameLBAWrite01) {
    lba = 30;
    lba_size = 10;

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba, VALID_HEX_DATA);
        parseAndExecute(cmd);
    }

    EXPECT_EQ(0, ssd->getAccessCount());
}

TEST_F(BufSSDTest, SameLBAWrite02) {
    lba = 20;
    lba_size = 10;

    for (int i = 0; i < 4; i++) {
        cmd = buildCommand("W", lba+i, PRECONDITION_HEX_DATA);
        parseAndExecute(cmd);
    }

    lba = 50;
    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba, VALID_HEX_DATA);
        parseAndExecute(cmd);
    }

    cmd = buildCommand("R", lba);
    parseAndExecute(cmd);
    EXPECT_TRUE(checkOutputFile(VALID_HEX_DATA));
    EXPECT_EQ(5, ssd->getAccessCount());
   
}

TEST_F(BufSSDTest, SameLBAWrite03) {
    lba = 20;
    lba_size = 5;

    //precondition
    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba + i, PRECONDITION_HEX_DATA);
        parseAndExecute(cmd);
    }

    lba = 21;

    for (int i = 0; i < 10; i++) {
        cmd = buildCommand("W", lba, VALID_HEX_DATA);
        parseAndExecute(cmd);
    }

    cmd = buildCommand("R", lba);
    parseAndExecute(cmd);
    EXPECT_TRUE(checkOutputFile(VALID_HEX_DATA));

    EXPECT_EQ(5, ssd->getAccessCount());
}

TEST_F(BufSSDTest, SameLBAWrite04) {
    lba = 20;
    lba_size = 6;

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba + i, VALID_HEX_DATA);
        parseAndExecute(cmd);
    }

    EXPECT_EQ(5, ssd->getAccessCount());
}

TEST_F(BufSSDTest, SameLBAWrite05) {
    lba = 20;
    lba_size = 10;

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba + i, VALID_HEX_DATA);
        parseAndExecute(cmd);
    }

    EXPECT_EQ(5, ssd->getAccessCount());
}

TEST_F(BufSSDTest, SameLBAWrite06) {
    lba = 20;
    lba_size = 11;

    //precondition
    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba + i, VALID_HEX_DATA);
        parseAndExecute(cmd);
    }

    EXPECT_EQ(10, ssd->getAccessCount());
}

TEST_F(BufSSDTest, EraseTest01) {
    lba = 20;
    lba_size = 1;

    //make precondition
    for (int i = 0; i < 5; i++) {
        cmd = buildCommand("W", lba + i, PRECONDITION_HEX_DATA);
        parseAndExecute(cmd);
    }

    cmd = buildCommand("R", lba);
    parseAndExecute(cmd);
    EXPECT_TRUE(checkOutputFile(PRECONDITION_HEX_DATA));
  
    cmd = buildCommand("E", lba, std::to_string(lba_size));
    parseAndExecute(cmd);
   
    cmd = buildCommand("R", lba);
    parseAndExecute(cmd);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(BufSSDTest, EraseTest02) {
    lba = 20;
    lba_size = 5;

    //make precondition
    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba + i, PRECONDITION_HEX_DATA);
        parseAndExecute(cmd);
    }

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("R", lba);
        parseAndExecute(cmd);
        EXPECT_TRUE(checkOutputFile(PRECONDITION_HEX_DATA));
    }

    cmd = buildCommand("E", lba, std::to_string(lba_size));
    parseAndExecute(cmd);

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("R", lba);
        parseAndExecute(cmd);
        EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
    }

    EXPECT_EQ(0, ssd->getAccessCount());
}


TEST_F(BufSSDTest, EraseTest03) {
    lba = 20;
    lba_size = 7;

    //make precondition
    for (int i = 0; i < 10; i++) {
        cmd = buildCommand("W", lba + i, PRECONDITION_HEX_DATA);
        parseAndExecute(cmd);
    }

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("R", lba);
        parseAndExecute(cmd);
        EXPECT_TRUE(checkOutputFile(PRECONDITION_HEX_DATA));
    }

    cmd = buildCommand("E", lba, std::to_string(lba_size));
    parseAndExecute(cmd);

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("R", lba);
        parseAndExecute(cmd);
        EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
    }

    EXPECT_EQ(10, ssd->getAccessCount());   //5 write + 5 erase
}

TEST_F(BufSSDTest, Erase_Exception) {
    lba = 94;
    lba_size = 2;

    //make precondition
    for (int i = 0; i < 5; i++) {
        cmd = buildCommand("W", lba + i, PRECONDITION_HEX_DATA);
        parseAndExecute(cmd);
    }

    lba = 99;
    cmd = buildCommand("E", lba, std::to_string(lba_size));
    parseAndExecute(cmd);

    EXPECT_TRUE(checkOutputFile("ERROR"));
}

#ifdef NDEBUG
int main(int argc, char *argv[])
{
    SSD* ssd = new BufferedSSD();
    string inputLine;

    // skip ssd.exe myself
    for (int i = 1; i < argc; ++i) {
        if (i > 1) inputLine += " ";
        inputLine += argv[i];
    }

    if (!ssd->parseCommand(inputLine))
        return -1;
    ssd->exec();

    return 0;
}
#else
int main()
{
    ::testing::InitGoogleMock();
    return RUN_ALL_TESTS();
}
#endif