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
bool valid(string);
