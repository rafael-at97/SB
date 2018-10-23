/*
 *	Trabalho 01 - Software Basico - Departamento de Ciencia da Computacao - Universidade de Brasilia
 *	Professor:	Bruno Machiavello
 *	Alunos:		Thiago Holanda - Matricula: XX/XXXXXXX 
 *				Rafael Torres  - Matricula: 15/0145365
 *		
 *	Implementation of an linker, which links various source code written in assembly into one 	
 *	The hypothetical machine has two registers, ACC (accumulator) and PC (program counter)
 *	The memory is made of 216 spaces of 16 bits, so, the machine can handle 16 bits words and instructions
 *	Table of instructions, opcodes and memory size for hypothetical machine can be seen of the table:
 *
 *	   |  Mnemonic  |   OpCode   |   Size    |         Action         |
 *	   |------------|------------|-----------|------------------------|
 *	   |    ADD	    |     01     |     2     | ACC <- ACC + mem(OP)   |
 *	   |	SUB		|	  02	 |	   2     | ACC <- ACC - mem(OP)	  |
 *	   |	MULT	|	  03	 |	   2     | ACC <- ACC x mem(OP)	  |
 *	   |	DIV		|	  04	 |	   2     | ACC <- ACC / mem(OP)	  |
 *	   |	JMP		|	  05	 |	   2     | PC  <- OP			  |
 *	   |	JMPN	|	  06	 |	   2     | ACC < 0 ? PC <- OP , ; |
 *	   |	JMPP	|	  07	 |	   2     | ACC > 0 ? PC <- OP , ; |
 *	   |	JMPZ	|	  08	 |	   2     | ACC = 0 ? PC <- OP , ; |
 *	   |	COPY	|	  09	 |	   3     | mem(OP2) <- mem(OP1)   |
 *	   |	LOAD	|	  10	 |	   2     | ACC <- mem(OP)    	  |
 *	   |    STORE	|	  11	 |	   2     | mem(OP) <- ACC         |
 *	   |	INPUT	|	  12	 |	   2     | mem(OP) <- input(STDIN)|
 *	   |	OUTPUT	|	  13	 |	   2     | output(STDOUT)<-mem(OP)|
 *	   |	STOP	|	  14	 |	   1     | halt code execution    |
 * 
 *	The program must generate an executable named 'ligador' - Portuguese for linker	
 *	It must take various codes as input from command line (something.obj) and create a (something.obj)
 *
 */

#include<cstdlib>
#include<iostream>
#include<fstream>
#include<string>
#include<list>
#include<map>
#include<vector>
#include"files.h"
#include"scanner.h"

list<pair<string, int> > t_use;

list<int> offs;

vector<int> code;

map<string, int> t_def;

// Need to deal with vectors between modules
map<int, int> offsets;

using namespace std;
 
bool verifica_tabela_uso(ifstream &file, int &line_number, bool &break_line){
	// Espera que esteja na linha 1
	
	string token;
	
	token = get_token(file, line_number, break_line);
	if(file.eof())
		return 0;
	
	if(!token.empty()){
		// Token is not empty
		if(line_number==1){
			if(token == "TABLE"){
				token = get_token(file, line_number, break_line);
				if(file.eof())
					return 0;
				
				if(!token.empty()){
					// Not empty
					if(line_number == 1){
						if(token == "USE"){
							return 1;
						};
					};
				};
			};
		};
	};
	
	return 0;
	
}

string get_label_and_arg(ifstream &file, int &line_number, bool &break_line, int offset){
	string token;
	int last_line = 1; // Assume last line was 1
	pair<string, int> p;
	
	token = get_token(file, line_number, break_line);
	if(file.eof())
		return "";
	
	while(line_number <= last_line+1){
		// Enquanto nao pulou uma linha vazia
		if(line_number == last_line){
			p.second = (strtol(token.c_str(), NULL, 10) + offset);
			t_use.push_back(p);
		}
		else {
			p.first = token;
		};
		last_line = line_number;
		if(!file.eof())
			token = get_token(file, line_number, break_line);
		else
			return "";
	};
	
	// Remember, it goes out and reads the token TABLE
	return token;
	
}
 
bool not_exist(string token){
	map<string, int>::iterator it;
	
	it = t_def.find(token);
	
	if(it!=t_def.end())
		return 0;
		
	return 1;	
} 
 
string get_label_def(ifstream &file, int &line_number, bool &break_line, int offset){
	string token, last_token;
	int last_line = line_number; // Assume last line was 1
	pair<string, int> p;
	
	if(file.eof())
		return "";
	
	token = get_token(file, line_number, break_line);
	
	while(line_number <= last_line+1){
		// Enquanto nao pulou uma linha vazia
		if(line_number == last_line){
			if(not_exist(last_token)){
				t_def.insert(pair<string, int>(last_token, strtol(token.c_str(), NULL, 10) + offset));
			};
		}
		else {
			last_token = token;
		};
		last_line = line_number;
		if(!file.eof())
			token = get_token(file, line_number, break_line);
		else
			return "";
	};
	
	// Remember, it goes out and reads the token TABLE
	return token;
} 
 
void print_list(){
	pair<string, int> p;
	
	while(!t_use.empty()){
		p = t_use.front();
		t_use.pop_front();
		cout << "Symbol: " << p.first << ", value: " << p.second << endl;
	};
} 
 
void print_map(){
	map<string, int>::iterator it;
	
	cout << "Definitions" << endl;
	
	for(it=t_def.begin();it!=t_def.end();it++){
		cout << "Symbol: " << it->first << ", value: " << it->second << endl;
	}
}  
 
bool verifica_tabela_def(ifstream &file, int &line_number, bool &break_line, string token){
	if(token != "TABLE"){
		return 0;
	};
	
	int last_line = line_number;
	
	if(file.eof())
		return 0;
		
	token = get_token(file, line_number, break_line);
	
	if(line_number!=last_line)
		return 0;
		
	if(token != "DEFINITION"){
		return 0;
	};	
	
	return 1;
} 
 
bool relatives(ifstream &file, int &line_number, bool &break_line, string &token){
	int last_line = line_number;
	
	if(token!="RELATIVE"){
		// Was expecting this token from past function
		return 0;
	};
	
	if(file.eof())
		return 0;
		
	token = get_token(file, line_number, break_line);
	
	while(line_number <= (last_line + 1)){
		offs.push_back(strtol(token.c_str(), NULL, 10));
		
		if(file.eof())
			return 0;
			
		token = get_token(file, line_number, break_line);
	};
	
	return 1;
} 

void print_relatives(){
	list<int>::iterator it;
	
	cout << "Relative:" << endl;
	
	while(!offs.empty()){
		cout << offs.front() << endl;
		offs.pop_front();
	}
	
}
 
bool fill_code(ifstream &file, int &line_number, bool &break_line, string token, int &offset){
	int i = 0;
	
	// Save position to be offseted
	int pos;
	if(!offs.empty()){
		pos = offs.front();
		offs.pop_front();
	}
	else {
		pos = -1;
	}
	
	if(token!="CODE"){
		// Was expecting this token from past function
		return 0;
	};
	
	if(file.eof())
		return 0;
		
	while(!file.eof()){
		token = get_token(file, line_number, break_line);
		
		if(!token.empty()){
			if(i == pos){
				offsets.insert(pair<int, int>(i+offset, strtol(token.c_str(), NULL, 10)));
				code.push_back(strtol(token.c_str(), NULL, 10) + offset);
				if(!offs.empty()){
					pos = offs.front();
					offs.pop_front();
				};
			}
			else {
				code.push_back(strtol(token.c_str(), NULL, 10));
			};
		i++;
		};
	}	
	
	offset += i;
	
	return 1;	
} 

void print_code(){
	cout << "code" << endl;

	for(int i=0;i<(int)code.size();i++){
		cout << code[i] << endl;
	}
}
 
int find_offset(int pos){
	map<int, int>::iterator it;
	
	it = offsets.find(pos);
	
	if(it!=offsets.end()){
		return it->second;
	}
	
	return 0;
} 
 
void insert_tables_values(){
	pair<string, int> my_pair;
	int pos;
	
	while(!t_use.empty()){
		my_pair = t_use.front();
		if(!not_exist(my_pair.first)){
			// Pega valor na tabela de definicao
			pos = t_def[my_pair.first];
			code[my_pair.second] = pos;
			code[my_pair.second] += find_offset(my_pair.second);
		}
		else {
			cout << "Error! Could not find definition for \"" << my_pair.first << "\"!" << endl;
		};
		t_use.pop_front();
	};
}
 
void print_to_file(string name){
	int i;
	ofstream output;
	
	// Change name of file to be written
	name = name.substr(0, name.find('.'));
	name += ".e";	
	
	if(!open_write(output, name)){
		return;
	};
	
	for(i=0;i<(int)code.size();i++){
		if(i){
			write(output, ' ' + to_string(code[i]));
		}
		else {
			write(output, to_string(code[i]));
		};
	};
	
	output.close();
} 
 
bool verify_and_fill(ifstream &file){
	string token;
	int line_number = 1;
	bool break_line = 0;
	
	while(!file.eof()){
		token = get_token(file, line_number, break_line);
		
		if(!token.empty()){
			if(line_number != 1 || !is_decimal(token, 0)){
				cout << "Not able to link this module by itself!" << endl;
				return 0;
			}
			else {
				code.push_back(strtol(token.c_str(), NULL, 10));
			};
		};
	};
	return 1;
} 
 
int main(int argc, char **argv){
	int i, line_number=1;
	int offset = 0;
	ifstream file;
	
	string token;
	
	bool break_line = 0;
	
	if(argc<2){
		cout << "Error! At least one input file expected!" << endl;
		return 0;
	}
	else if(argc == 2){
		// Opened correctly
		if(!open_and_check(file, string(argv[1]), ".obj"))
			return 0;
			
		// Copy the input file and change it's name to extension '.e'
		if(!verify_and_fill(file))	
			return 0;
	}
	else {
		// Read all inputs
		for(i=1;i<argc;i++){
			// Check extension and if it opened correctly
			if(!open_and_check(file, string(argv[i]), ".obj"))
				return 0;
			
			// File opened correctly
			if(verifica_tabela_uso(file, line_number, break_line)){ 
				token = get_label_and_arg(file, line_number, break_line, offset);	
			}
			else {
				cout << "Error, could not find TABLE USE in module " << i << endl;
				break;
			};
			if(verifica_tabela_def(file, line_number, break_line, token)){
				token = get_label_def(file, line_number, break_line, offset);
			}
			else {
				cout << "Error, could not find TABLE DEFINITION in module " << i << endl;
			};
			if(!relatives(file, line_number, break_line, token)){
				cout << "Error, could not find information for relatives in module " << i << endl;
			};
			if(!fill_code(file, line_number, break_line, token, offset)){
				cout << "Error, could not find information for code in module " << i << endl;
			};
			// Close last file
			file.close();	
			line_number = 1;
			break_line = 0;
		};
	};
	
	insert_tables_values();
	print_to_file(string(argv[1]));

	/*	
	print_list();
	print_map();
	print_relatives();
	print_code();
	*/
	
	return 0;
	
}
