#include<iostream>
#include <fstream>
#include <string>
#include <iomanip>

using namespace std;
using std::string;

class SSD {
public:
	void commandParser(string command) {

	}
	int readData(int address) {

        string data[100][20];
        //generate init ssd_nand file
        if ((address >= 0) && (address < 100))
        {
            //ofstream fout;
            ofstream fout("ssd_nand.txt");

            //if(!fout){}
            //if(fout.is_open()){}

            for (int i = 0; i < 100; i++) {
                string data;
                fout << dec << i << "\t" << "0x" << hex << std::setw(8) << std::setfill('0') << i << "\n";

            }
            fout.close();

            ifstream fin("ssd_nand.txt");
          //  string line;
            for (int i = 0; i < 100; i++) {
                
                fin.getline(data[i], 20);
               // fin.getline(fout, 20);
                    //fout.getloc(i);//getline
                cout << data[i];
          
            }

            fin.close();
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