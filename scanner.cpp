/*	
 *	Library to deal with scanner methods
 */

#include<string>
#include<fstream>
#include<cctype>
#include"scanner.h" 
 
using namespace std; 
 
// Name self-explanatory
bool is_ignore(char c){
	 return (c=='\n' || c=='\t' || c == ' ');
}

// Name self-explanatory
string upper(string str){
	int i;
	for(i=0;i<(int)str.length();i++){
		str[i] = toupper(str[i]);	
	}
	
	return str;
}

// Get tokens from source file
string get_token(ifstream &file){
	char c = ' ';
	string token = "";
	
	// While the char is space or tab or space, keep reading
	while(is_ignore(c) && !file.eof()){
		file.get(c);
		
		// Deals with comments, read until next line
		if(c==';'){
			while(c!='\n' && !file.eof())
				file.get(c);	
		};
	}
	
	// The past loop makes sure the char is a valid one, read and append until a not valid char is found
	while(!file.eof() && !is_ignore(c)){
		token.append(&c);
		file.get(c);
		
		// Deals with comments, read until next line
		if(c==';'){
			while(c!='\n' && !file.eof())
				file.get(c);
			
			break;	
		}
		else if(c==':'){
			// Token is a rotule definition, append the ':' symbol
			token.append(&c);
			break;
		};
		
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
			
		if(!(isalpha(str[i]) || isdigit(str[i]) || str[i]=='_'))
			return 0;
	};
	
	return 1;
}

bool is_signed(string str){
	return (str[0] == '-' || str[0] == '+');
}

bool is_decimal(string str){
	int length = (int)str.length();
	int i;

	if(is_signed(str)){
		// Avoids iterating through the first element
		
		// if length is under 2 and is signed no number was found
		if(length < 2)
			return 0;

		for(i=1;i<length;i++){
			if(!isdigit(str[i])){
				return 0;
			};
		};	
	}
	else {
		for(i=0;i<length;i++){
			if(!isdigit(str[i])){
				return 0;
			};
		};
	};
	
	return 1;	
}

bool is_hexadecimal(string str){
	int length = (int)str.length();
	int i;

	if(is_signed(str)){
		// Avoids iterating through the first element
		
		// if length is under 4 and is signed no number was found (-0X...)
		if(length < 4)
			return 0;

		// Did not find "0X" initializing hexadecimal value
		if(str[1] != '0' && toupper(str[2]) != 'X')
			return 0;

		for(i=4;i<length;i++){
			if(!isdigit(str[i])){
				return 0;
			};
		};	
	}
	else {
		// if length is under 3 no number was found (0X...)
		if(length < 3)
			return 0;

		// Did not find "0X" initializing hexadecimal value
		if(str[0] != '0' && toupper(str[1]) != 'X')
			return 0;

		for(i=3;i<length;i++){
			if(!isdigit(str[i])){
				return 0;
			};
		};	
	};
	
	return 1;	
}
