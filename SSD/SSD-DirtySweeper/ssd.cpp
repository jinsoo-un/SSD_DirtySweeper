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
        if ((address < 0) || (address >= 100))
        {
            //Error
            return -1;
        }

        MakeTempFile();     // generate tempfile for test

        ifstream fin("ssd_nand.txt");
        ofstream fout2("ssd_output.txt");
            
        char line[20];

        for (int i = 0; i < 100; i++) {
            if (i == address)
            {
                fin.getline(line, 20);
 //               cout << line << "\n";
            }
                
        }
        fin.close();
        fout2.close();
        return 0;
      
	}
   
	void writeData(int address, int data) {

	}

    void MakeTempFile()
    {
        ofstream fout("ssd_nand.txt");

        for (int i = 0; i < 100; i++) {
            string data;
            fout << dec << i << "\t" << "0x" << hex << std::setw(8) << std::setfill('0') << i << "\n";

        }
        fout.close();
    }

};