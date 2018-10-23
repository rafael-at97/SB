/*	
 *	Library to deal with file stream specific verifications
 */

#include<fstream>
#include<string>
 
using namespace std;

bool open_and_check(ifstream &, string, string);

bool open_write(ofstream &, string);

bool open_pre(ifstream &, string);

void write_newl(ofstream &);

void write(ofstream &output, string);
