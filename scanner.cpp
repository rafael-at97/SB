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

bool is_int(string str){
	int x = str.length();

	for(int i=1;i<x;i++){
		if(!isdigit(str[i]))
			return 0;
	};
	
	// Special case, user inserted sign
	if(str[0]=='+' || str[0]=='-'){
		if(x==1)			  // If length is only one, input has no number
			return 0;
	}		
	else if(!isdigit(str[0])) // Last check, if it is not a digit	
		return 0;
	
	return 1;
}

int strtoi(string str){
	int x = str.length(), base = 1, sum=0;

	for(int i=x-1;i>=0;i--){
		if(!i){
			if(str[i] == '-'){
				sum = -sum;
				break;
			}
			else if(str[i] == '+')
				break;	
		};
		sum += (str[i]-'0')*base;
		base*=10;
	};
	
	return sum;
}
