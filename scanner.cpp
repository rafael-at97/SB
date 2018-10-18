/*	
 *	Library to deal with scanner methods
 */

#include<string>
#include<fstream>
#include"scanner.h" 
 
using namespace std; 
 
// Name self-explanatory
bool is_valid(char c){
	 return ((c == ' ' || c == '\n' || c== '\t') ? 0 : 1);
}

// Name self-explanatory
string upper(string str){
	int i;
	for(i=0;i<(int)str.length();i++){
		if(str[i]>='a' && str[i]<='z')
			str[i] = str[i] - 'a' + 'A';	
	}
	
	return str;
}

// Get tokens from source file
string get_token(ifstream &file){
	char c = ' ';
	string token;
	
	while(!is_valid(c) && !file.eof()){
		file.get(c);
		
		// Deals with comments, read until next line
		if(c==';'){
			while(c!='\n' && !file.eof())
				file.get(c);
			// Read once more, avoid 'c' maintaining '\n' value
			file.get(c);	
		};
	}
	
	while(!file.eof() && c!=' ' && c!='\n' && c!='\t'){
		token.append(&c);
		file.get(c);
		
		// Deals with comments, read until next line
		if(c==';'){
			while(c!='\n' && !file.eof())
				file.get(c);
			// Read once more, avoid 'c' maintaining '\n' value
			file.get(c);
			break;	
		};		
		
	};
	
	// If anything was inserted into the string
	if(token.length()){
		// Not sure if needed
		c = '\0';
		token.append(&c);	
	};
	
	return token;
}

// Check if token is a valid token
bool valid(string str){
	int x = str.length();
	if(x > 50)
		return 0;
		
	int i;
		
	for(i=0;i<x;i++){
		if(!i && isdigit(str[i]))
			return 0;
			
		if(!isdigit(str[i]) && !isalpha(str[i]) && (str[i] != '_'))
			return 0;
	};
	
	return 1;
}
