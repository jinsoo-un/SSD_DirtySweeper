
#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>

using namespace std;
using std::string;

class SSD {
public:
	void commandParser(string command) {
		std::istringstream iss(command);
		string arg;
		int cnt = 0;

		/* scan the command line input */
		while (iss >> arg) {
			cnt++;
			if (cnt == 1)
				checkOp(arg);
			if (cnt == 2)
				addr = std::stoi(arg);
			if (cnt == 3)
				value = arg;
		}
		argCount = cnt;
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

	void checkOp(string arg) {
		if (arg != "R" && arg != "W")
			throw std::exception();
		op = arg;
	}

	int argCount;
	string op;
	int addr;
	string value;
};