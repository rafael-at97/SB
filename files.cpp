/*	
 *	Library to deal with file stream specific verifications
 */

#include<fstream>
#include<string>
#include<iostream>
#include"files.h" 
 
using namespace std; 
 
bool open_and_check(ifstream &source, string file_name){
	// Check if .asm extension and show warning if not found
	if(file_name.substr(file_name.length()-4, file_name.length()) != ".asm"){
		cout << "Warning! Expected assembly file extensions (*.asm), continuing anyway." << endl;
	};

	// Open the source code file and verifies it is really open. OBS: 'open' function needs c string format to open file
	source.open(file_name.c_str());
	if(!source.is_open()) {
		cout << "Could not find or open source file!" << endl;
		return 0;
	};
	
	return 1;
}
