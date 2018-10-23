/*	
 *	Library to deal with file stream specific verifications
 */

#include<fstream>
#include<string>
#include<iostream>
#include"files.h" 
 
using namespace std; 
 
bool open_and_check(ifstream &source, string file_name, string expected){
	// Check if correct extension and show warning if not found
	if(file_name.substr(file_name.length()-4, file_name.length()) != expected){
		cout << "Warning! Expected file extension of type (" << expected << "), continuing anyway." << endl;
	};

	// Open the source code file and verifies it is really open. OBS: 'open' function needs c string format to open file
	source.open(file_name.c_str());
	if(!source.is_open()) {
		cout << "Could not find or open source file!" << endl;
		return 0;
	};
	
	return 1;
}

bool open_write(ofstream &output, string file_name){
	// Open the source code file and verifies it is really open. OBS: 'open' function needs c string format to open file
	output.open(file_name.c_str());
	if(!output.is_open()) {
		cout << "Could not find or open source file!" << endl;
		return 0;
	};
	
	return 1;
}

bool open_pre(ifstream &source, string file_name){
	// Open the source code file and verifies it is really open. OBS: 'open' function needs c string format to open file
	source.open(file_name.c_str());
	if(!source.is_open()) {
		cout << "Could not find or open pre-processed file! Something went wrong during pre-processing!" << endl;
		return 0;
	};
	
	return 1;
}

void write_newl(ofstream &output){
	output << endl;
}

void write(ofstream &output, string str){
	output << str;
}
