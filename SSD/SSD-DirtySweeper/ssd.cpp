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

        //generate init ssd_nand file
        if ((address < 0) || (address >= 100)){
            //Error
            string msg{ "ERROR" };
            ofstream fout2("ssd_output.txt");
            fout2 << msg;
            fout2.close();
            return -1;
        }


        ifstream fin("ssd_nand.txt");
        ofstream fout2("ssd_output.txt");
            
        char line[20];

        for (int i = 0; i < 100; i++) {
            fin.getline(line, 20);

            if (i == address)
            {
                fout2 << line;
            }                
        }

        fin.close();
        fout2.close();
        return 0;
      
	}
   
	void writeData(int address, int data) {

	}
};