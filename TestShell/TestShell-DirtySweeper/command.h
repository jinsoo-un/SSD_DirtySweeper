#pragma once
#include "logger.h"
#include "testShell_output_manager.h"
#include "file_accessor.h"
#include "ssd.h"

class Command {
public:
	Command(SSD* ssd) : ssd{ ssd } {}
	virtual string execute() = 0;

protected:
	SSD* ssd;
};

class ReadCommand : public Command {
public:
	ReadCommand(SSD* ssd, int lba) : Command(ssd), lba{ lba } {}
	string execute() override;

private:
	int lba;
};

class FullReadCommand : public Command {
public:
	FullReadCommand(SSD* ssd) : Command(ssd) {}
	string execute() override;
private:
	const int LBA_START_ADDRESS = 0;
	const int LBA_END_ADDRESS = 99;
};