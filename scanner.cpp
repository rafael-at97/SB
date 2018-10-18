/*	
 *	Library to deal with scanner methods
 */

#include<string>
#include<fstream>
#include"scanner.h" 
 
using namespace std; 
 
// Name self-explanatory
bool is_valid(char c){
	 return (isdigit(c) || isalpha(c) || (c == '_'));
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
	string token = "";
	
	// While the char is not valid, keep reading
	while(!is_valid(c) && !file.eof()){
		file.get(c);
		
		// Deals with comments, read until next line
		if(c==';'){
			while(c!='\n' && !file.eof())
				file.get(c);	
		};
	}
	
	// The past loop makes sure the char is a valid one, read and append until a not valid char is found
	while(!file.eof() && is_valid(c)){
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
