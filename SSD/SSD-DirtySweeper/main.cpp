#include <iostream>
#include "ssd.cpp"
#include "gmock/gmock.h"

using std::string;

class SSDTest : public ::testing::Test {
public:
    SSD ssd;
    ReadCommand readCmd;
    WriteCommand writeCmd;
    EraseCommand eraseCmd;
    string VALID_HEX_DATA = "0x1298CDEF";
    string INVALID_HEX_DATA = "0xABCDEFGH";
    string INITIAL_HEX_DATA = "0x00000000";
    static const int VALID_TEST_ADDRESS = 0;
    static const int INVALID_TEST_ADDRESS = 100;

    void SetUp() override {
        eraseCmd.run();
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

    string buildCommand(string rw, int lba, string data = "") {
        string cmdLine = rw + " " + std::to_string(lba);
        if (rw == "W") cmdLine = cmdLine + " " + data;
        return cmdLine;
    }
};


TEST_F(SSDTest, ReadTC_InitialValue)
{
    int lba_addr = 1;
    bool isPass;
    isPass = readCmd.run(lba_addr);
    EXPECT_EQ(true, isPass);
	EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(SSDTest, ReadTC_OutofRange)
{
    int lba_addr = 100;
    bool isPass;
    isPass = readCmd.run(lba_addr);
    EXPECT_EQ(false, isPass);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(SSDTest, ReadTC_ReturnData01)
{
    int lba_addr = 50;
    bool isPass;
    isPass = readCmd.run(lba_addr);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(SSDTest, ReadTC_ReturnData02)
{
    int lba_addr = 30;
    bool isPass;
    isPass = readCmd.run(lba_addr);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));
}

TEST_F(SSDTest, ArgparseRead) {
    string cmd = buildCommand("R", 3);
    ssd.parseCommand(cmd);
    EXPECT_EQ(2, ssd.getArgCount());
    EXPECT_EQ("R", ssd.getOp());
    EXPECT_EQ(3, ssd.getAddr());
}

TEST_F(SSDTest, ArgparseWrite)
{
    string cmd = buildCommand("W", 3, VALID_HEX_DATA);
    ssd.parseCommand(cmd);
    EXPECT_EQ(3, ssd.getArgCount());
    EXPECT_EQ("W", ssd.getOp());
    EXPECT_EQ(3, ssd.getAddr());
    EXPECT_EQ("0x1298CDEF", ssd.getValue());
}

TEST_F(SSDTest, ArgparseInvalidOp)
{
    string cmd = buildCommand("S", 3);
    ssd.parseCommand(cmd);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(SSDTest, ArgparseInvalidAddr)
{
    string cmd = buildCommand("R", 300);
    ssd.parseCommand(cmd);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(SSDTest, ArgparseInvalidValue)
{
    string cmd = buildCommand("W", 3, INVALID_HEX_DATA);
    ssd.parseCommand(cmd);
    EXPECT_TRUE(checkOutputFile("ERROR"));
}

TEST_F(SSDTest, WritePass) {
    bool isPass = writeCmd.run(VALID_TEST_ADDRESS, VALID_HEX_DATA);
    EXPECT_TRUE(isPass);
}

TEST_F(SSDTest, WriteFailWithOutOfAddressRange) {
    bool isPass = writeCmd.run(INVALID_TEST_ADDRESS, VALID_HEX_DATA);
    EXPECT_FALSE(isPass);
}


TEST_F(SSDTest, WriteInvalidData00) {

    string invalidData = "0x1234567890000";
    bool isPass = writeCmd.run(VALID_TEST_ADDRESS, invalidData);
    EXPECT_FALSE(isPass);
}

TEST_F(SSDTest, WriteInvalidData01) {

    string invalidData = "0x1234";
    bool isPass = writeCmd.run(VALID_TEST_ADDRESS, invalidData);
    EXPECT_FALSE(isPass);
}

TEST_F(SSDTest, WriteInvalidData02) {

    string invalidData = "12345678";
    bool isPass = writeCmd.run(VALID_TEST_ADDRESS, invalidData);
    EXPECT_FALSE(isPass);
}

TEST_F(SSDTest, WriteInvalidData03) {

    string invalidData = "0x1234ABzE";
    bool isPass = writeCmd.run(VALID_TEST_ADDRESS, invalidData);
    EXPECT_FALSE(isPass);
}

TEST_F(SSDTest, WriteInvalidData04) {

    string invalidData = "0xA5CCH012";
    bool isPass = writeCmd.run(VALID_TEST_ADDRESS, invalidData);
    EXPECT_FALSE(isPass);
}

TEST_F(SSDTest, WriteReadVerify00) {

    bool isPass = readCmd.run(VALID_TEST_ADDRESS);

    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(INITIAL_HEX_DATA));

    isPass = writeCmd.run(VALID_TEST_ADDRESS, VALID_HEX_DATA);
    EXPECT_TRUE(isPass);

    isPass = readCmd.run(VALID_TEST_ADDRESS);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(VALID_HEX_DATA));
}
///////////////////////////////////////////////////////////////////////////

TEST_F(SSDTest, SameLBAWrite01) {
    int access_count;
    int writeLBA = 30;
    bool isPass;

    for(int i=0; i<10; i++){
        isPass = writeCmd.run(writeLBA, VALID_HEX_DATA);
        EXPECT_TRUE(isPass);
    }

    access_count = ssd.getAccessCount();
    EXPECT_EQ(access_count, 0);

    isPass = readCmd.run(writeLBA);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(VALID_HEX_DATA));
}

TEST_F(SSDTest, SameLBAWrite02) {
    int access_count;
    int writeLBA = 20;
    bool isPass;

    //precondition
    for (int i = 0; i < 4; i++) {
        isPass = writeCmd.run(writeLBA+i, "0x123454678");
        EXPECT_TRUE(isPass);
    }

    writeLBA = 50;
    for (int i = 0; i < 10; i++) {
        isPass = writeCmd.run(writeLBA, VALID_HEX_DATA);
        EXPECT_TRUE(isPass);
    }

    isPass = readCmd.run(writeLBA);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(VALID_HEX_DATA));

    access_count = ssd.getAccessCount();
    EXPECT_EQ(access_count, 0);
}

TEST_F(SSDTest, SameLBAWrite03) {
    int access_count;
    int writeLBA = 20;
    bool isPass;

    //precondition
    for (int i = 0; i < 5; i++) {
        isPass = writeCmd.run(writeLBA + i, "0x123454678");
        EXPECT_TRUE(isPass);
    }

    writeLBA = 21;
    for (int i = 0; i < 10; i++) {
        isPass = writeCmd.run(writeLBA, VALID_HEX_DATA);
        EXPECT_TRUE(isPass);
    }

    isPass = readCmd.run(writeLBA);
    EXPECT_EQ(true, isPass);
    EXPECT_TRUE(checkOutputFile(VALID_HEX_DATA));

    access_count = ssd.getAccessCount();
    EXPECT_EQ(access_count, 0);
}

TEST_F(SSDTest, SameLBAWrite04) {
    int access_count;
    int writeLBA = 20;
    bool isPass;

    //precondition
    for (int i = 0; i < 6; i++) {
        isPass = writeCmd.run(writeLBA + i, "0x123454678");
        EXPECT_TRUE(isPass);
    }

    access_count = ssd.getAccessCount();
    EXPECT_EQ(access_count, 5);
}

TEST_F(SSDTest, SameLBAWrite05) {
    int access_count;
    int writeLBA = 20;
    bool isPass;

    //precondition
    for (int i = 0; i < 10; i++) {
        isPass = writeCmd.run(writeLBA + i, "0x123454678");
        EXPECT_TRUE(isPass);
    }

    access_count = ssd.getAccessCount();
    EXPECT_EQ(access_count, 5);
}

TEST_F(SSDTest, SameLBAWrite06) {
    int access_count;
    int writeLBA = 20;
    bool isPass;

    //precondition
    for (int i = 0; i < 11; i++) {
        isPass = writeCmd.run(writeLBA+i, "0x123454678");
        EXPECT_TRUE(isPass);
    }

    access_count = ssd.getAccessCount();
    EXPECT_EQ(access_count, 5);
}


#ifdef NDEBUG
int main(int argc, char *argv[])
{
    SSD ssd;
    string inputLine;

    // skip ssd.exe myself
    for (int i = 1; i < argc; ++i) {
        if (i > 1) inputLine += " ";
        inputLine += argv[i];
    }

    if (!ssd.parseCommand(inputLine))
        return -1;
    ssd.exec();

    return 0;
}
#else
int main()
{
    ::testing::InitGoogleMock();
    return RUN_ALL_TESTS();
}
#endif