#include<iostream>
#include <fstream>

using std::string;

class SSD {
public:
	void commandParser(string command) {

	}
	int readData(int address) {

        if ((address >= 0) && (address < 100))
        {
            return 0;
        }
        else {
            return -1;
        }
        
       //char ssd_file = "ssd_nand.txt";
       //char file="ssd_output.txt"

		/*
		   const char* filename = "sample.txt";

    // Open the file in read/write binary mode
    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);

    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return 1;
    }

    // Move to the 5th byte (index 4)
    file.seekp(4, std::ios::beg);  // for writing at offset 4

    if (!file) {
        std::cerr << "Failed to seek to position." << std::endl;
        return 1;
    }

    // Write a new character at that position
    file.put('X');

    if (!file) {
        std::cerr << "Failed to write to file." << std::endl;
        return 1;
    }

    file.close();
    std::cout << "Successfully modified byte at position 4." << std::endl;

    return 0;
		*/

	}
	void writeData(int address, int data) {

	}
};