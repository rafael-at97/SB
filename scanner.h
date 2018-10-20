/*	
 *	Library to deal with scanner methods
 */
 
#include<string>
#include<fstream> 
 
using namespace std; 
 
// Name self-explanatory
bool is_valid(char);

// Name self-explanatory
string upper(string);

// Get tokens from source file
string get_token(ifstream &);

// Check if token is a valid token
bool valid(string &, short int);

// Check if it is signed
bool is_signed(string);

// Check if string represents a decimal
bool is_decimal(string);

// Check if string represents a hexadecimal
bool is_hexadecimal(string);
