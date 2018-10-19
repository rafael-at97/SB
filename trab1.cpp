/*
 *	Trabalho 01 - Software Basico - Departamento de Ciencia da Computacao - Universidade de Brasilia
 *	Professor:	Bruno Machiavello
 *	Alunos:		Thiago Holanda - Matricula: XX/XXXXXXX 
 *				Rafael Torres  - Matricula: 15/0145365
 *		
 *	Implementation of an assembler, which translates a source code written in assembly for a hypothetical machine to machine code	
 *	The hypothetical machine has two registers, ACC (accumulator) and PC (program counter)
 *	The memory is made of 216 spaces of 16 bits, so, the machine can handle 16 bits words and instructions
 *	Table of instructions, opcodes and memory size for hypothetical machine can be seen of the table:
 *
 *	   |  Mnemonic  |   OpCode   |   Size    |         Action         |
 *	   |------------|------------|-----------|------------------------|
 *	   |    ADD	    |     01     |     2     | ACC <- ACC + mem(OP)   |
 *	   |	SUB		|	  02	 |	   2     | ACC <- ACC - mem(OP)	  |
 *	   |	MUL		|	  03	 |	   2     | ACC <- ACC x mem(OP)	  |
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
 *	The program must generate an executable named 'montador' - Portuguese for assembler	
 *	It must take a code as input from command line (something.asm) and create a (something.obj)
 *
 * 	Current version:
 *		V0.0 - Assembler simply capable of reading input source code and translate mnemonics to opcode
 *			 - Implemented using single-pass algorithm to solve the forward-reference problem
 *		V1.0 - Fully implements symbol table and solved forward-reference problem
 *			 - Presents lexical and syntax errors	
 *		V2.0 - Accept directives SPACE and CONST, as well as the arguments + and constant for SPACE
 *			 - Trying to implement possibility of using (+) when calling labels, ex: ADD: N + 5
 */ 

#include<cstdlib>
#include<iostream>
#include<map>
#include<set>
#include<string>
#include<fstream>
#include<vector>
#include"files.h"
#include"scanner.h"

using namespace std;

// Table of instructions, ordered to make it easier to find mnemonics
map<string, int> instructions;

// Table of symbols
/*			Example
 *	Name | Value  | Defined
 *    A  |    2   |    0
 *    B  |    1   |    1 
 *
 */
map<string, pair<int, bool> > symbols;

// Directives table
set<string> directives;

// Vectors para codigos (opcode e espacos na memoria) e informacao de codigos relativos
vector<int> code;
vector<int> relatives;

void fill_instruction_and_directives_table(){
	// Insertion of instructions in the table
	instructions.insert( pair<string, int>("ADD", 1) );
	instructions.insert( pair<string, int>("SUB", 2) );
	instructions.insert( pair<string, int>("MUL", 3) );
	instructions.insert( pair<string, int>("DIV", 4) );
	instructions.insert( pair<string, int>("JMP", 5) );
	instructions.insert( pair<string, int>("JMPN", 6) );
	instructions.insert( pair<string, int>("JMPP", 7) );
	instructions.insert( pair<string, int>("JMPZ", 8) );
	instructions.insert( pair<string, int>("COPY", 9) );
	instructions.insert( pair<string, int>("LOAD", 10) );
	instructions.insert( pair<string, int>("STORE", 11) );
	instructions.insert( pair<string, int>("INPUT", 12) );
	instructions.insert( pair<string, int>("OUTPUT", 13) );
	instructions.insert( pair<string, int>("STOP", 14) );	

	// Fill directives table
	directives.insert("SPACE");
	directives.insert("CONST");
}

bool is_instruction(string token){
	// Using iterator to avoid using [] operator and inserting new values into the instructions map structure
	map<string, int>::iterator map_it;
	
	map_it = instructions.find(upper(token));

	if(map_it != instructions.end()){
		// The token is a instruction
		return 1;
	}
	else
		return 0; 
}

bool is_label_def(string &token){
	if(token[token.length() - 1] == ':'){
		token = token.substr(0, token.length()-1);
		return 1;
	};
	return 0;
}

bool is_directive(string token){
	set<string>::iterator set_it;
	
	set_it = directives.find(token);
	
	if(set_it != directives.end())
		return 1;
		
	return 0;	
}

short int already_defined(string token){
	// Using iterator to avoid using [] operator and inserting new values into the instructions map structure
	map<string, pair<int, bool> >::iterator symbol_it;
	
	symbol_it = symbols.find(token);

	if(symbol_it != symbols.end()){
		// The token is a instruction
		
		// Return 0 or 1, depends if it was already defined
		return (symbol_it->second).second;
	}
	else
		return -1;
}

void recursive_definition(string token, int pos){
	int _pos, tmp;
	
	_pos = symbols[token].first;
	
	do {
		tmp = code[_pos];
		code[_pos] = pos;
		_pos = tmp;
	} while(_pos!=-1);
}

// This function creates a sequence of the numerical code that represents the source code
void assemble(ifstream &source){
	string token, last_instruction="";
	int pos = 0;
	
	bool flag_end = 0, flag_read = 1;	
	short int cnt = 0;

	while(!source.eof()){
		// Get next token if did not already when checking directives
		if(flag_read)
			token = get_token(source);
		
		// If there is a token
		if(!token.empty()){
			// Check if token is a instruction, label definition or none (assume label)
			if(is_instruction(token)){
			
				if(cnt){
					cout << "Error! Missing arguments for instruction " << last_instruction << "!" << endl;
				};

				if(flag_end){
					cout << "Error! Stop already detected, cannot handle any more instructions!!" << endl;
				};
				
				token = upper(token);
				
				if(token == "STOP"){
					flag_end = 1;
					cnt = 0;
				}
				else if(token == "COPY"){
					cnt = 2;
				}
				else {
					cnt = 1;
				};
				
				last_instruction = token;
				
				code.push_back(instructions[token]);
				pos++;
			}
			else if(is_directive(token)){
				if(cnt){
					cout << "Error! Missing arguments for instruction " << last_instruction << "!" << endl;
				};
				
				token = upper(token);
				
				if(token == "SPACE"){
					// Get a new token and check if it is a plus sign (+)
					token = get_token(source);
					
					if(token == "+"){
						// The user wants to allocate more than a single memory space
						// Get a new token and check if it is a number
						
						token = get_token(source);
						
						if(is_int(token)){
							// Must insert strtoi(token) + 1 positions filled with 0 in the code
							code.insert(code.end(), (strtoi(token)+1), 0);
							pos += (strtoi(token)+1);
						}
						else {
							// Not a number as expected
							cout << "Could not resolve constant for SPACE directive!" << endl;
							
							// Tells the assembler not to read
							flag_read = 0;
							continue;
						};
					}
					else {
						// Allocates a single memory space
						code.push_back(0);
						pos++;
						
						// Tells the assembler not to overwrite the already written token
						flag_read = 0;
						continue;
					};
				}
				else if(token == "CONST"){
					// Get a new token and check if it is a numerical string
					token = get_token(source);
					
					if(is_int(token)){
						// Correct argument for directive, add data to the code and read next token normally on next loop
						code.push_back(strtoi(token));
						pos++;
					}
					else {
						// Incorrect argument for directive, may be instruction, label definition or something else
						// Set a flag so don't read another token and overwrite this one
						cout << "Could not resolve constant for CONST directive!" << endl;
						flag_read = 0;
						continue;
					};
				};
			}
			else if(is_label_def(token)){
				// If is label definition, the checking function already erases the ':'
				if(valid(token)){
					// Token is valid
					
					if(cnt){
						cout << "Error! Missing arguments for instruction " << last_instruction << "!" << endl;
					};
					
					token = upper(token);
					
					short int ver = already_defined(token);
					
					// Already defined
					if(ver == 1){
						cout << ver << endl;
						cout << symbols[token].first << endl;
					}
					else if(!ver){
						// Already exists, but not defined
						
						recursive_definition(token, pos);
						
						// CHANGE THE FIRST VALUE TO THE POSITION IN THE CODE
						symbols[token].first = pos;
						
						// Set as defined
						symbols[token].second = 1;
					}
					else {
						// -1, does not even exist!
						
						// CHANGE POSITION TO THE COUNTER IN THE CODE
						symbols[token] = make_pair(pos, 1);
					};
					
				}
				else {
					cout << "Invalid label definition: \"" << token << "\"!" << endl;
				};
			}
			else {
				// Assume it is a label
				if(valid(token)){
					// Token is valid
					
					if(!cnt){
						// Save last instruction
						cout << "Error! Too many arguments for instruction " << last_instruction << "!" << endl;
					}
					else {
						token = upper(token);
					
						short int ver = already_defined(token); 	
				
						if(ver == 1){
							code.push_back(symbols[token].first);
						}
						else if(!ver){
							code.push_back(symbols[token].first);
							symbols[token].first = pos;
						}
						else {
							code.push_back(-1);
							symbols[token] = make_pair(pos, 0);
						};
						
						relatives.push_back(pos);
						
						cnt--;
						pos++;
					};
				}
				else {
					cout << "Invalid token: \"" << token << "\"!" << endl;
				};
			};
		};
		// Reset some flags
		flag_read = 1;
	};

}

void print_code(){
	int i;
	
	cout << "RELATIVE" << endl;
	for(i=0;i<(int)relatives.size();i++)
		if(!i)
			cout << relatives[i];
		else
			cout << ' ' << relatives[i];	
		
	cout << endl << endl;	
		
	cout << "CODE" << endl;
	for(i=0;i<(int)code.size();i++)
		if(!i)
			cout << code[i];
		else
			cout << ' ' << code[i];	
		
	cout << endl;		

}

int main(int argc, char **argv){
	// Check correct number of arguments, 2 as the first is the program name	
	if(argc != 2){
		cout << "Expected 1 input source code, halting assembler!" << endl;
		return 0;
	};

	// The virtual file that represents the physical file in the HD
	ifstream source;
	
	// Check extension and if it opened correctly
	if(!open_and_check(source, string(argv[1])))
		return 0;

	// Fill the instruction table for future queries
	fill_instruction_and_directives_table();

	// Function that reads the input file and fills the 'code' and 'relative' vectors, most important function in the code
	assemble(source);

	// Print the results as expecified
	print_code();
		
	//cout << instructions["ADD"] << endl;
	return 0;
}
