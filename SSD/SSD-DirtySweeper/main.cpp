#include <iostream>
#include "ssd.cpp"
#include "gmock/gmock.h"

using std::string;

class RealSSDTest : public ::testing::Test {
public:
    SSD* ssd = new RealSSD();
    ReadCommand readCmd;
    WriteCommand writeCmd;
    EraseCommand eraseCmd;
    string VALID_HEX_DATA = "0x1298CDEF";
    string INVALID_HEX_DATA = "0xABCDEFGH";
    string INITIAL_HEX_DATA = "0x00000000";
    static const int VALID_TEST_ADDRESS = 0;
    static const int VALID_TEST_ADDRESS_MAX = 99;
    static const int INVALID_TEST_ADDRESS = 100;
    static const int VALID_TEST_SIZE = 10;
    static const int INVALID_TEST_SIZE = 20;


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
};

TEST_F(RealSSDTest, ReadTC_InitialValue)
{
    int lba_addr = 1;
    bool isPass;
    isPass = readCmd.run(lba_addr);
    EXPECT_EQ(true, isPass);
	EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(RealSSDTest, ReadTC_OutofRange)
{
    int lba_addr = 100;
    bool isPass;
    isPass = readCmd.run(lba_addr);
    EXPECT_EQ(false, isPass);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(RealSSDTest, ReadTC_ReturnData01)
{
    int lba_addr = 50;
    bool isPass;
    isPass = readCmd.run(lba_addr);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(RealSSDTest, ReadTC_ReturnData02)
{
    int lba_addr = 30;
    bool isPass;
    isPass = readCmd.run(lba_addr);
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
    bool isPass = writeCmd.run(VALID_TEST_ADDRESS, VALID_HEX_DATA);
    EXPECT_TRUE(isPass);
}

TEST_F(RealSSDTest, WriteFailWithOutOfAddressRange) {
    bool isPass = writeCmd.run(INVALID_TEST_ADDRESS, VALID_HEX_DATA);
    EXPECT_FALSE(isPass);
}


TEST_F(RealSSDTest, WriteInvalidData00) {

    string invalidData = "0x1234567890000";
    bool isPass = writeCmd.run(VALID_TEST_ADDRESS, invalidData);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteInvalidData01) {

    string invalidData = "0x1234";
    bool isPass = writeCmd.run(VALID_TEST_ADDRESS, invalidData);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteInvalidData02) {

    string invalidData = "12345678";
    bool isPass = writeCmd.run(VALID_TEST_ADDRESS, invalidData);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteInvalidData03) {

    string invalidData = "0x1234ABzE";
    bool isPass = writeCmd.run(VALID_TEST_ADDRESS, invalidData);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteInvalidData04) {

    string invalidData = "0xA5CCH012";
    bool isPass = writeCmd.run(VALID_TEST_ADDRESS, invalidData);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteReadVerify00) {

    bool isPass = readCmd.run(VALID_TEST_ADDRESS);

    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));

    isPass = writeCmd.run(VALID_TEST_ADDRESS, VALID_HEX_DATA);
    EXPECT_TRUE(isPass);

    isPass = readCmd.run(VALID_TEST_ADDRESS);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(VALID_HEX_DATA));
}

TEST_F(RealSSDTest, ErasePass) {
    bool isPass = eraseCmd.run(VALID_TEST_ADDRESS, VALID_HEX_DATA, VALID_TEST_SIZE);
    EXPECT_TRUE(isPass);
}

TEST_F(RealSSDTest, EraseFailOutofRange) {
    bool isPass = eraseCmd.run(INVALID_TEST_ADDRESS, VALID_HEX_DATA, VALID_TEST_SIZE);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, EraseFailOutofRangeDestination) {
    bool isPass = eraseCmd.run(VALID_TEST_ADDRESS_MAX, VALID_HEX_DATA, VALID_TEST_SIZE);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, EraseFailExceedMaxSize) {
    bool isPass = eraseCmd.run(VALID_TEST_ADDRESS, VALID_HEX_DATA, INVALID_TEST_SIZE);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, EraseAndReadVerify) {
    bool isPass = writeCmd.run(VALID_TEST_ADDRESS, VALID_HEX_DATA);
    EXPECT_TRUE(isPass);

    isPass = eraseCmd.run(VALID_TEST_ADDRESS, VALID_HEX_DATA, VALID_TEST_SIZE);
    EXPECT_TRUE(isPass);

    isPass = readCmd.run(VALID_TEST_ADDRESS);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

///////////////////////////////////////////////////////////////////////////
class BufSSDTest : public ::testing::Test {
public:
    SSD* ssd = new BufferedSSD();

    string VALID_HEX_DATA = "0x1298CDEF";
    string INVALID_HEX_DATA = "0xABCDA5A5";
    string INITIAL_HEX_DATA = "0x00000000";

    static const int VALID_TEST_ADDRESS = 0;
    static const int VALID_TEST_ADDRESS_MAX = 99;
    static const int INVALID_TEST_ADDRESS = 100;
    static const int VALID_TEST_SIZE = 10;
    static const int INVALID_TEST_SIZE = 20;
    
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
};

TEST_F(BufSSDTest, SameLBAWrite01) {
    lba = 30;
    lba_size = 10;

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba, VALID_HEX_DATA);
        ssd->parseCommand(cmd);
        ssd->exec();
    }

    EXPECT_EQ(0, ssd->getAccessCount());
}

TEST_F(BufSSDTest, SameLBAWrite02) {
    lba = 20;
    lba_size = 10;

    for (int i = 0; i < 4; i++) {
        cmd = buildCommand("W", lba+i, INVALID_HEX_DATA);
        ssd->parseCommand(cmd);
        ssd->exec();
    }

    lba = 50;
    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba, VALID_HEX_DATA);
        ssd->parseCommand(cmd);
        ssd->exec();
    }

    cmd = buildCommand("R", lba);
    ssd->parseCommand(cmd);
    ssd->exec();
    EXPECT_TRUE(checkOutputFile(VALID_HEX_DATA));
    EXPECT_EQ(0, ssd->getAccessCount());
   
}

TEST_F(BufSSDTest, SameLBAWrite03) {
    lba = 20;
    lba_size = 5;

    //precondition
    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba + i, INVALID_HEX_DATA);
        ssd->parseCommand(cmd);
        ssd->exec();
    }

    lba = 21;

    for (int i = 0; i < 10; i++) {
        cmd = buildCommand("W", lba, VALID_HEX_DATA);
        ssd->parseCommand(cmd);
        ssd->exec();
    }

    cmd = buildCommand("R", lba);
    ssd->parseCommand(cmd);
    ssd->exec();
    EXPECT_TRUE(checkOutputFile(VALID_HEX_DATA));

    EXPECT_EQ(0, ssd->getAccessCount());
}

TEST_F(BufSSDTest, SameLBAWrite04) {
    lba = 20;
    lba_size = 6;

    //precondition
    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba + i, VALID_HEX_DATA);
        ssd->parseCommand(cmd);
        ssd->exec();;
    }

    EXPECT_EQ(5, ssd->getAccessCount());
}

TEST_F(BufSSDTest, SameLBAWrite05) {
    lba = 20;
    lba_size = 10;

    //precondition
    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba + i, VALID_HEX_DATA);
        ssd->parseCommand(cmd);
        ssd->exec();
    }

    EXPECT_EQ(5, ssd->getAccessCount());
}

TEST_F(BufSSDTest, SameLBAWrite06) {
    lba = 20;
    lba_size = 11;

    //precondition
    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba + i, VALID_HEX_DATA);
        ssd->parseCommand(cmd);
        ssd->exec();
    }

    EXPECT_EQ(10, ssd->getAccessCount());
}

TEST_F(BufSSDTest, EraseTest01) {
    lba = 20;
    lba_size = 1;

    //make precondition
    for (int i = 0; i < 5; i++) {
        cmd = buildCommand("W", lba + i, INVALID_HEX_DATA);
        ssd->parseCommand(cmd);
        ssd->exec();
    }

    cmd = buildCommand("R", lba);
    ssd->parseCommand(cmd);
    ssd->exec();
    EXPECT_TRUE(checkOutputFile(INVALID_HEX_DATA));
  
    cmd = buildCommand("E", lba, std::to_string(lba_size));
    ssd->parseCommand(cmd);
    ssd->exec();
   
    cmd = buildCommand("R", lba);
    ssd->parseCommand(cmd);
    ssd->exec();
    EXPECT_TRUE(checkOutputFile(INVALID_HEX_DATA));
}

TEST_F(BufSSDTest, EraseTest02) {
    lba = 20;
    lba_size = 5;

    //make precondition
    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba + i, VALID_HEX_DATA);
        ssd->parseCommand(cmd);
        ssd->exec();
    }

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("R", lba);
        ssd->parseCommand(cmd);
        ssd->exec();
        EXPECT_TRUE(checkOutputFile(VALID_HEX_DATA));
    }

    cmd = buildCommand("E", lba, std::to_string(lba_size));
    ssd->parseCommand(cmd);
    ssd->exec();

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("R", lba);
        ssd->parseCommand(cmd);
        ssd->exec();
        EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
    }

    EXPECT_EQ(0, ssd->getAccessCount());
}


TEST_F(BufSSDTest, EraseTest03) {
    lba = 20;
    lba_size = 7;

    //make precondition
    for (int i = 0; i < 10; i++) {
        cmd = buildCommand("W", lba + i, VALID_HEX_DATA);
        ssd->parseCommand(cmd);
        ssd->exec();
    }

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("R", lba);
        ssd->parseCommand(cmd);
        ssd->exec();
        EXPECT_TRUE(checkOutputFile(VALID_HEX_DATA));
    }

    cmd = buildCommand("E", lba, std::to_string(lba_size));
    ssd->parseCommand(cmd);
    ssd->exec();

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("R", lba);
        ssd->parseCommand(cmd);
        ssd->exec();
        EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
    }

    EXPECT_EQ(10, ssd->getAccessCount());   //5 write + 5 erase
}

TEST_F(BufSSDTest, Erase_Exception) {
    lba = 94;
    lba_size = 2;

    //make precondition
    for (int i = 0; i < 5; i++) {
        cmd = buildCommand("W", lba + i, VALID_HEX_DATA);
        ssd->parseCommand(cmd);
        ssd->exec();
    }

    lba = 99;
    cmd = buildCommand("E", lba, std::to_string(lba_size));
    ssd->parseCommand(cmd);
    ssd->exec();

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