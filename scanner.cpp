/*	
 *	Library to deal with scanner methods
 */

#include<string>
#include<fstream>
#include<cctype>
#include<algorithm>
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
string get_token(ifstream &file, int &line_number, bool &break_line){
	char c = ' ';
	string token = "";
	
	if(break_line){
		line_number++;
		break_line = 0;
	};
	
	// While the char is space or tab or space, keep reading
	while(is_ignore(c) && !file.eof()){
		file.get(c);
		
		if(c == '\n')
			line_number++;
		
		// Deals with comments, read until next line
		if(c==';'){
			while(c!='\n' && !file.eof()){
				file.get(c);
				if(c == '\n')
					line_number++;	
			};		
		};
	}
	
	// The past loop makes sure the char is a valid one, read and append until a not valid char is found
	while(!file.eof() && !is_ignore(c)){
		token.append(&c);
		file.get(c);
		
		if(c == '\n'){
			break_line = 1;
			break;
		};
		
		// Deals with comments, read until next line
		if(c==';'){
			while(c!='\n' && !file.eof()){
				file.get(c);
				if(c == '\n')
					break_line = 1;
			};
			
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
bool valid(string& str, short int cnt){
	int x = str.length();
	if(x > 50)
		return 0;
		
	int i;
		
	for(i=0;i<x;i++){
		// If begins with number
		if(!i && isdigit(str[i]))
			return 0;
			
		// Check if last char is a ','
		if(i == (x-1)){
			// For COPY, only the first argument receives a ',', so the counter will still be 2
			if(cnt == 2){
				if(str[i] == ',' && x>1){	// x+1 avoids taking ',' alone as valid
					// Accepts ',' as last char, but takes it out
					str = str.substr(0, x-1);
					return 1;
				};
			};
		};	
			
		if(!(isalpha(str[i]) || isdigit(str[i]) || str[i]=='_'))
			return 0;
	};
	
	return 1;
}

bool is_signed(string str){
	return (str[0] == '-' || str[0] == '+');
}

bool is_decimal(string& str, short int cnt){
	int length = (int)str.length();
	int i;

	if(is_signed(str)){
		// Avoids iterating through the first element
		
		// if length is under 2 and is signed no number was found
		if(length < 2)
			return 0;

		for(i=1;i<length;i++){
			if(i == length-1){
				if(cnt){
					// Accepts ',' as part of int
					if(str[i] == ',' && length>2){
						str = str.substr(0, length-1);
						return 1;
					};
				};
			};
			
			if(!isdigit(str[i])){
				return 0;
			};
		};	
	}
	else {
		for(i=0;i<length;i++){
			if(i == length-1){
				if(cnt){
					// Accepts ',' as part of int
					if(str[i] == ',' && length>1){
						str = str.substr(0, length-1);
						return 1;
					};
				};
			};
					
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

string to_string(int num){
	string tmp = "";
	bool positive = 1;
	int result;
	
	if(num<0){
		positive = 0;
		num = -num;
	};

	while(num>0){
		result = num%10;
		tmp += (char)(result + '0');
		num = num/10;
	};
	
	if(tmp == ""){
		tmp += '0';
	};

	if(!positive){
		tmp += '-';
	};

	reverse(tmp.begin(), tmp.end());

	return tmp;
}
