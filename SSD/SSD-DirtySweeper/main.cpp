#include <iostream>
#include "ssd.cpp"
#include "gmock/gmock.h"
#include <memory>

using namespace std;

namespace SSDTestMaterials {
    string VALID_HEX_DATA = "0x1298CDEF";
    string INVALID_HEX_DATA = "0xABCDEFGH";
    string INITIAL_HEX_DATA = "0x00000000";
    string PRECONDITION_HEX_DATA = "0xABCDA5A5";
    
    static const int VALID_TEST_ADDRESS = 0;
    static const int VALID_TEST_ADDRESS_MAX = 99;
    static const int INVALID_TEST_ADDRESS = 100;
    static const int VALID_TEST_SIZE = 10;
    static const int INVALID_TEST_SIZE = 20;
}
using namespace SSDTestMaterials;


void initializeDataFile()
{
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
    if (cmd == "F") cmdLine = "F";
    return cmdLine;
}

class ParserTest : public ::testing::Test {
public:
    CommandParser parser;
    string cmd;
    commandParams param;

    void SetUp() override {
        initializeDataFile();
    }

};

TEST_F(ParserTest, ArgparseRead) {
    cmd = buildCommand("R", 3);
    parser.parseCommand(cmd, param);

    EXPECT_EQ("R", param.op);
    EXPECT_EQ(3, param.addr);
}

TEST_F(ParserTest, ArgparseWrite)
{
    cmd = buildCommand("W", 3, VALID_HEX_DATA);
    parser.parseCommand(cmd, param);
    EXPECT_EQ("W", param.op);
    EXPECT_EQ(3, param.addr);
    EXPECT_EQ("0x1298CDEF", param.value);
}

TEST_F(ParserTest, ArgparseErase)
{
    cmd = buildCommand("E", 3, std::to_string(10));
    parser.parseCommand(cmd, param);
    EXPECT_EQ("E", param.op);
    EXPECT_EQ(3, param.addr);
    EXPECT_EQ(10, param.size);
}

TEST_F(ParserTest, ArgparseInvalidOp)
{
    cmd = buildCommand("S", 3);
    parser.parseCommand(cmd, param);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(ParserTest, ArgparseInvalidAddr)
{
    cmd = buildCommand("R", 300);
    parser.parseCommand(cmd, param);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(ParserTest, ArgparseInvalidValue)
{
    cmd = buildCommand("W", 3, INVALID_HEX_DATA);
    parser.parseCommand(cmd, param);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}


///////////////////////////////////////////////////////////////////////////
class RealSSDTest : public ::testing::Test {
public:
    RealSSDTest() : ssd {new RealSSD } { }
    SSD* ssd;
    string cmd;

    void SetUp() override {
        initializeDataFile();
    }
};

TEST_F(RealSSDTest, ReadTC_InitialValue)
{
    cmd = buildCommand("R", 1);
    bool isPass = ssd->execute(cmd);

    EXPECT_EQ(true, isPass);
	EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(RealSSDTest, ReadTC_OutofRange)
{
    cmd = buildCommand("R", 100);
    bool isPass = ssd->execute(cmd);
    EXPECT_EQ(false, isPass);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(RealSSDTest, ReadTC_ReturnData01)
{
    cmd = buildCommand("R", 50);
    bool isPass = ssd->execute(cmd);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(RealSSDTest, ReadTC_ReturnData02)
{
    cmd = buildCommand("R", 30);
    bool isPass = ssd->execute(cmd);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(RealSSDTest, WritePass) {
    cmd = buildCommand("W", VALID_TEST_ADDRESS, VALID_HEX_DATA);
    bool isPass = ssd->execute(cmd);
    EXPECT_TRUE(isPass);
}

TEST_F(RealSSDTest, WriteFailWithOutOfAddressRange) {
    cmd = buildCommand("W", INVALID_TEST_ADDRESS, VALID_HEX_DATA);
    bool isPass = ssd->execute(cmd);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteInvalidData00) {
    cmd = buildCommand("W", VALID_TEST_ADDRESS, "0x1234567890000");
    bool isPass = ssd->execute(cmd);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteInvalidData01) {
    cmd = buildCommand("W", VALID_TEST_ADDRESS, "0x1234");
    bool isPass = ssd->execute(cmd);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteInvalidData02) {
    cmd = buildCommand("W", VALID_TEST_ADDRESS, "12345678");
    bool isPass = ssd->execute(cmd);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteInvalidData03) {
    cmd = buildCommand("W", VALID_TEST_ADDRESS, "0x1234ABzE");
    bool isPass = ssd->execute(cmd);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteInvalidData04) {
    cmd = buildCommand("W", VALID_TEST_ADDRESS, "0xA5CCH012");
    bool isPass = ssd->execute(cmd);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, WriteReadVerify00) {
    cmd = buildCommand("R", VALID_TEST_ADDRESS);
    bool isPass = ssd->execute(cmd);

    ASSERT_EQ(true, isPass);
    ASSERT_TRUE(checkOutputFile(INITIAL_HEX_DATA));

    cmd = buildCommand("W", VALID_TEST_ADDRESS, VALID_HEX_DATA);
    isPass = ssd->execute(cmd);
    ASSERT_TRUE(isPass);

    cmd = buildCommand("R", VALID_TEST_ADDRESS);
    isPass = ssd->execute(cmd);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(VALID_HEX_DATA));
}

TEST_F(RealSSDTest, ErasePass) {
    cmd = buildCommand("E", VALID_TEST_ADDRESS, to_string(VALID_TEST_SIZE));
    bool isPass = ssd->execute(cmd);
    EXPECT_TRUE(isPass);
}

TEST_F(RealSSDTest, EraseFailOutofRange) {
    cmd = buildCommand("E", INVALID_TEST_ADDRESS, to_string(VALID_TEST_SIZE));
    bool isPass = ssd->execute(cmd);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, EraseFailOutofRangeDestination) {
    cmd = buildCommand("E", VALID_TEST_ADDRESS_MAX, to_string(VALID_TEST_SIZE));
    bool isPass = ssd->execute(cmd);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, EraseFailExceedMaxSize) {
    cmd = buildCommand("E", VALID_TEST_ADDRESS, to_string(INVALID_TEST_SIZE));
    bool isPass = ssd->execute(cmd);
    EXPECT_FALSE(isPass);
}

TEST_F(RealSSDTest, EraseAndReadVerify) {
    cmd = buildCommand("W", VALID_TEST_ADDRESS, VALID_HEX_DATA);
    bool isPass = ssd->execute(cmd);
    ASSERT_TRUE(isPass);

    cmd = buildCommand("E", VALID_TEST_ADDRESS, to_string(VALID_TEST_SIZE));
    isPass = ssd->execute(cmd);
    ASSERT_TRUE(isPass);

	cmd = buildCommand("R", VALID_TEST_ADDRESS);
    isPass = ssd->execute(cmd);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

///////////////////////////////////////////////////////////////////////////
class BufSSDTest : public RealSSDTest {
public:
    BufSSDTest() : ssd { new BufferedSSD } { }
    SSD* ssd;
    Buffer buffer;
    string cmd;

    int lba;
    int lba_size;

    void SetUp() override {
        initializeDataFile();
        buffer.clear();
    }

};

TEST_F(BufSSDTest, ReadTC_InitialValue)
{
    cmd = buildCommand("R", 1);
    bool isPass = ssd->execute(cmd);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(BufSSDTest, ReadTC_OutofRange)
{
    cmd = buildCommand("R", 100);
    bool isPass = ssd->execute(cmd);
    EXPECT_EQ(false, isPass);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(BufSSDTest, ReadTC_ReturnData01)
{
    cmd = buildCommand("R", 50);
    bool isPass = ssd->execute(cmd);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(BufSSDTest, ReadTC_ReturnData02)
{
    cmd = buildCommand("R", 30);
    bool isPass = ssd->execute(cmd);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(BufSSDTest, SameLBAWrite01) {
    lba = 30;
    lba_size = 10;

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba, VALID_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    EXPECT_EQ(0, ssd->getAccessCount());
}

TEST_F(BufSSDTest, SameLBAWrite02) {
    lba = 20;
    lba_size = 10;

    for (int i = 0; i < 4; i++) {
        cmd = buildCommand("W", lba+i, PRECONDITION_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    lba = 50;
    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba, VALID_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    cmd = buildCommand("R", lba);
    EXPECT_EQ(true, ssd->execute(cmd));
    EXPECT_TRUE(checkOutputFile(VALID_HEX_DATA));
    EXPECT_EQ(5, ssd->getAccessCount());
   
}

TEST_F(BufSSDTest, SameLBAWrite03) {
    lba = 20;
    lba_size = 5;

    //precondition
    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba + i, PRECONDITION_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    lba = 21;

    for (int i = 0; i < 10; i++) {
        cmd = buildCommand("W", lba, VALID_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    cmd = buildCommand("R", lba);
    EXPECT_EQ(true, ssd->execute(cmd));
    EXPECT_TRUE(checkOutputFile(VALID_HEX_DATA));

    EXPECT_EQ(5, ssd->getAccessCount());
}

TEST_F(BufSSDTest, SameLBAWrite04) {
    lba = 20;
    lba_size = 6;

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba + i, VALID_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    EXPECT_EQ(5, ssd->getAccessCount());
}

TEST_F(BufSSDTest, SameLBAWrite05) {
    lba = 20;
    lba_size = 10;

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba + i, VALID_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    EXPECT_EQ(5, ssd->getAccessCount());
}

TEST_F(BufSSDTest, SameLBAWrite06) {
    lba = 20;
    lba_size = 11;

    //precondition
    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba + i, VALID_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    EXPECT_EQ(10, ssd->getAccessCount());
}

TEST_F(BufSSDTest, EraseTest01) {
    lba = 20;
    lba_size = 1;

    //make precondition
    for (int i = 0; i < 5; i++) {
        cmd = buildCommand("W", lba + i, PRECONDITION_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    cmd = buildCommand("R", lba);
    EXPECT_EQ(true, ssd->execute(cmd));
    EXPECT_TRUE(checkOutputFile(PRECONDITION_HEX_DATA));
  
    cmd = buildCommand("E", lba, std::to_string(lba_size));
    EXPECT_EQ(true, ssd->execute(cmd));;
   
    cmd = buildCommand("R", lba);
    EXPECT_EQ(true, ssd->execute(cmd));
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(BufSSDTest, EraseTest02) {
    lba = 20;
    lba_size = 5;

    //make precondition
    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba + i, PRECONDITION_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("R", lba);
        EXPECT_EQ(true, ssd->execute(cmd));;
        EXPECT_TRUE(checkOutputFile(PRECONDITION_HEX_DATA));
    }

    cmd = buildCommand("E", lba, std::to_string(lba_size));
    EXPECT_EQ(true, ssd->execute(cmd));

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("R", lba);
        EXPECT_EQ(true, ssd->execute(cmd));
        EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
    }

    EXPECT_EQ(5, ssd->getAccessCount());
}


TEST_F(BufSSDTest, EraseTest03) {
    lba = 20;
    lba_size = 7;

    //make precondition
    for (int i = 0; i < 10; i++) {
        cmd = buildCommand("W", lba + i, PRECONDITION_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("R", lba + i);
        EXPECT_EQ(true, ssd->execute(cmd));
        EXPECT_TRUE(checkOutputFile(PRECONDITION_HEX_DATA));
    }

    cmd = buildCommand("E", lba, std::to_string(lba_size));
    EXPECT_EQ(true, ssd->execute(cmd));

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("R", lba + i);
        EXPECT_EQ(true, ssd->execute(cmd));
        EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
    }

    EXPECT_EQ(10, ssd->getAccessCount());   //5 write + 5 erase
}

TEST_F(BufSSDTest, EraseException) {
    lba = 94;
    lba_size = 2;

    //make precondition
    for (int i = 0; i < 5; i++) {
        cmd = buildCommand("W", lba + i, PRECONDITION_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    lba = 99;
    cmd = buildCommand("E", lba, std::to_string(lba_size));
    EXPECT_EQ(false, ssd->execute(cmd));

    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(BufSSDTest, EraseMergeEraseTest) {
    lba = 30;
    cmd = buildCommand("E", lba, std::to_string(2));
    EXPECT_EQ(true, ssd->execute(cmd));

    cmd = buildCommand("E", lba + 2, to_string(3));
    EXPECT_EQ(true, ssd->execute(cmd));

    lba = 40;
    for (int i = 0; i < 4; i++) {
        cmd = buildCommand("W", lba + i, PRECONDITION_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    // If erase are merged, flush should not be called;
    EXPECT_EQ(0, ssd->getAccessCount());
}

TEST_F(BufSSDTest, EraseMergeOutOfRangeTest) {
    lba = 30;
    cmd = buildCommand("E", lba, std::to_string(5));
    EXPECT_EQ(true, ssd->execute(cmd));

    cmd = buildCommand("E", lba + 4, to_string(8));
    EXPECT_EQ(true, ssd->execute(cmd));

    lba = 40;
    for (int i = 0; i < 4; i++) {
        cmd = buildCommand("W", lba + i, PRECONDITION_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    // If erase are merged, flush should not be called;
    // if erase are not merged, flush should be called;
    EXPECT_EQ(16, ssd->getAccessCount());
}

TEST_F(BufSSDTest, EraseMergeSingleWriteTest) {
    lba = 30;
    cmd = buildCommand("W", lba, PRECONDITION_HEX_DATA);
    EXPECT_EQ(true, ssd->execute(cmd));

    cmd = buildCommand("E", lba, to_string(3));
    EXPECT_EQ(true, ssd->execute(cmd));

    lba = 40;
    for (int i = 0; i < 4; i++) {
        cmd = buildCommand("W", lba + i, PRECONDITION_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    // If erase are merged, flush should not be called;
    EXPECT_EQ(0, ssd->getAccessCount());
}

TEST_F(BufSSDTest, EraseMergeMultipleWriteTest) {
    lba = 30;
    for (int i = 0; i < 3; i++) {
        cmd = buildCommand("W", lba + i, PRECONDITION_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    cmd = buildCommand("E", lba, to_string(3));
    EXPECT_EQ(true, ssd->execute(cmd));

    lba = 40;
    for (int i = 0; i < 4; i++) {
        cmd = buildCommand("W", lba + i, PRECONDITION_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    // If erase are merged, flush should not be called;
    EXPECT_EQ(0, ssd->getAccessCount());
}

// In case of order "E W E", last "E" cannot merge the "E" before "W"
// if overlapped
TEST_F(BufSSDTest, EraseNotMergeEraseBeforeWriteTest) {
    lba = 30;
    cmd = buildCommand("E", lba, to_string(3)); // erase 30, 31, 32
    EXPECT_EQ(true, ssd->execute(cmd));

    cmd = buildCommand("W", lba, PRECONDITION_HEX_DATA); // write 30
    EXPECT_EQ(true, ssd->execute(cmd));

    cmd = buildCommand("E", lba + 2, to_string(3)); // erase 32, 33, 34
    EXPECT_EQ(true, ssd->execute(cmd));

    cmd = buildCommand("R", lba); // if read 30, must not be erased
    EXPECT_EQ(true, ssd->execute(cmd));
    EXPECT_TRUE(checkOutputFile(PRECONDITION_HEX_DATA));
}

TEST_F(BufSSDTest, Flush01) {
    lba = 30;
    lba_size = 5;

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba + i, VALID_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    EXPECT_EQ(0, ssd->getAccessCount());

    cmd = buildCommand("F", 0, VALID_HEX_DATA);
    EXPECT_EQ(true, ssd->execute(cmd));

    EXPECT_EQ(5, ssd->getAccessCount());
}

TEST_F(BufSSDTest, Flush02) {
    lba = 30;
    lba_size = 10;

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba + i, VALID_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    EXPECT_EQ(5, ssd->getAccessCount());

    cmd = buildCommand("F", 0, VALID_HEX_DATA);
    EXPECT_EQ(true, ssd->execute(cmd));

    EXPECT_EQ(10, ssd->getAccessCount());
}

TEST_F(BufSSDTest, Flush03) {
    lba = 30;
    lba_size = 4;

    for (int i = 0; i < lba_size; i++) {
        cmd = buildCommand("W", lba + i, VALID_HEX_DATA);
        EXPECT_EQ(true, ssd->execute(cmd));
    }

    EXPECT_EQ(0, ssd->getAccessCount());

    cmd = buildCommand("E", lba, std::to_string(2));
    EXPECT_EQ(true, ssd->execute(cmd));

    cmd = buildCommand("F", 0, VALID_HEX_DATA);
    EXPECT_EQ(true, ssd->execute(cmd));

    EXPECT_EQ(4, ssd->getAccessCount());
}

TEST_F(BufSSDTest, OutOfRange) {
    lba = 10;
    lba_size = 10;

    cmd = buildCommand("E", lba, std::to_string(lba_size));
    EXPECT_EQ(true, ssd->execute(cmd));

    lba = 20;
    lba_size = 15;
    cmd = buildCommand("E", lba, std::to_string(lba_size));
    EXPECT_EQ(false, ssd->execute(cmd));


    lba = -1;    
    cmd = buildCommand("W", lba, VALID_HEX_DATA);
    EXPECT_EQ(false, ssd->execute(cmd));

    lba = 100;
    cmd = buildCommand("W", lba, VALID_HEX_DATA);
    EXPECT_EQ(false, ssd->execute(cmd));

    cmd = buildCommand("R", lba, VALID_HEX_DATA);
    EXPECT_EQ(false, ssd->execute(cmd));

    cmd = buildCommand("S", lba, VALID_HEX_DATA);
    EXPECT_EQ(false, ssd->execute(cmd));

    cmd = buildCommand("F", 0, VALID_HEX_DATA);
    EXPECT_EQ(true, ssd->execute(cmd));
}
#ifdef NDEBUG
int main(int argc, char *argv[])
{
    SSD* ssd = new BufferedSSD();
    string ssdCommand;

    // skip ssd.exe myself
    for (int i = 1; i < argc; ++i) {
        if (i > 1) ssdCommand += " ";
        ssdCommand += argv[i];
    }

    if (!ssd->execute(ssdCommand))
        return -1;
    
    return 0;
}
#else
int main()
{
    ::testing::InitGoogleMock();
    return RUN_ALL_TESTS();
}
#endif