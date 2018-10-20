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
 *		V2.0 - Accept directives SPACE and CONST, as well as arguments
 *		V2.1 - Accept/treat negative values for SPACE and CONST
 *		V2.2 - Minor fixes on detecting number: a + or - by itself is not a number!
 *	    V2.3 - Trying to accept hexadecimal as number!
 *  	V2.4 - Trying to implement possibility of using (+) when calling labels, ex: ADD: N + 5
 *	->	V2.5 - Need to detect if a ',' is used between COPY instruction
 *		V2.6 - Cannot use certain instructions involving constants 
 *		V3.0 - Accept directives SECTION
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

// Table that helps deal with offsets
map<int, int> offsets;

// Directives table
set<string> directives;

// Vectors to code (opcode and memory spaces) and information on relative positions
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

// Simply check if the token is in the map that contains the instructions
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

// Check if the last char is the ':' char and erases it of the string
bool is_label_def(string &token){
	if(token[token.length() - 1] == ':'){
		token = token.substr(0, token.length()-1);
		return 1;
	};
	return 0;
}

// Check it the token is present in the set of directives
bool is_directive(string token){
	set<string>::iterator set_it;
	
	set_it = directives.find(upper(token));
	
	if(set_it != directives.end())
		return 1;
		
	return 0;	
}


// Access the symbol map and if exists: return 1 -> already defined, 0-> not defined. It it does not exist return -1
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

// Check if a position is in the offsets mapping data structure and returns the value of the offset
int check_offset(int pos){
	map<int, int>::iterator it;
	
	it = offsets.find(pos);
	
	if(it!=offsets.end()){
		// If a result was found
		int value = it->second;
		
		// Erases from the data structure, given that position was already used
		offsets.erase(it);
		
		return value;
	}
	else {
		return 0;
	};
	
	return 0;
}

// Does a 'recursive' fill in the code vector of the positions of a previously unknown label
void recursive_definition(string token, int pos){
	int _pos, tmp;
	
	// Save the position pointed by the symbols table
	_pos = symbols[token].first;
	
	do {
		// Avoid losing reference to the position pointed in the code
		tmp = code[_pos];
		
		// Update the position in the code
		code[_pos] = pos;
		
		// Check if it needs offsets
		code[_pos] += check_offset(_pos);
		
		// 'Recursive' update of the position value
		_pos = tmp;
		
	} while(_pos!=-1);
}

// Does what need to be done when an instruction is found
string solve_instruction(string token, bool *flags, short int &cnt){
	token = upper(token);
	
	// If STOP, activate flag end to mean that code has ended
	if(token == "STOP"){
		flags[0] = 1;
		cnt = 0;
	}
	else if(token == "COPY"){
		cnt = 2;
	}
	else {
		cnt = 1;
	};
	
	// Save the opcode in the code
	code.push_back(instructions[token]);
	
	// Returns the instruction to be saved
	return token;
}

// Until now, returns nothing
void solve_directive(string token, bool *flags){
	// First, make sure token is uppercase
	token = upper(token);				
					
	// Specific cases, SPACE and CONST				
	if(token == "SPACE"){
		// Check a flag and tells the assembler it is waiting for an argument
		flags[1] = 1;
	}
	else if(token == "CONST"){
		// Check a flag and tells the assembler it is waiting for an argument
		flags[2] = 1;
	};
}

// Check most flags and solve some problems 0: No error, 1: Argument errors, 2: STOP error
short int check_problems(short int &cnt, bool *flags, string last_instruction, int &pos, short int type, string token){

	// If is an instruction but STOP was already found
	if(type == 0 && flags[0]){
		cout << "Error! Stop already detected, cannot handle any more instructions!!" << endl;
		return 2;
	};

	if(flags[1]){
		// Was expecting an argument for SPACE
		// If not found, simply puts a 0 in the code and increment memory position
		code.push_back(0);
		pos++;
		
		// Reset flag
		flags[1] = 0;
		
		// Waiting for SPACE arguments and not finding any is not an error
		// return 1;
	};
	
	// If cnt is different from 0, means that an argument label was expected, print error and reset cnt
	if(cnt && type!=3){
		cout << "Error! Missing arguments for instruction " << last_instruction << "!" << endl;
		
		// Reset counter so the next instruction can update the counter accordingly
		cnt = 0;
		return 1;
	}
	else if(!cnt && type == 3){
		cout << "Error! Too many arguments for instruction " << last_instruction << "!" << endl;
		return 1;
	}
	
	if(flags[2]){
		// Was expecting an argument for CONST directive
		// If not found, print error
		cout << "Error! Expected argument for \"CONST\" directive!" << endl;
		
		// Does not increment 'pos' because the const was not declared correctly
		
		// Reset flag because error was already printed
		flags[2] = 0;
		return 1;
	};
	if(flags[3]){
		// Was expecting instruction or directives (SPACE and CONST only)
		
		// Reset flag
		flags[3] = 0;
		
		if(type!=0 && type!=1){
			return 1;
		}
		else{
			if(type == 1 && token != "CONST" && token != "SPACE")
				return 1;
		};	
	};
	if(flags[4]){
		// Plus sign was expected, not found, simply reset flag
		flags[4] = 0;
	};
	if(flags[5]){
		// Expecting argument for '+' sign, not found means error
		cout << "Error! Missing argument for '+' operand!" << endl;
		
		// Reset flag
		flags[5] = 0;
		return 1;
	};
	
	// No problems found
	return 0;
}

// Solve labels definition
void solve_label_def(string token, bool *flags, int pos){
	token = upper(token);
	
	// 1: Exists and defined, 0: Exists but not defined, -1: Does not exist
	short int ver = already_defined(token);
	
	// Already defined
	if(ver == 1){
		cout << "Error! Redefinition of label!" << endl;
		return;
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
	
	// Tell assembler it is expecting an instruction or some directives (SPACE and CONST)
	flags[3] = 1;
}

// Deals with labels
string solve_label(string token, int pos, bool *flags){
	token = upper(token);

	// 1: Exists and defined, 0: Exists but not defined, -1: Does not exist
	short int ver = already_defined(token); 

	if(ver == 1){
		// Already exists and defined, simply insert into the code the equivalent memory location
		code.push_back(symbols[token].first);
	}
	else if(!ver){
		// Isn't defined yet
		code.push_back(symbols[token].first);
		symbols[token].first = pos;
	}
	else {
		// First time seeing label
		code.push_back(-1);
		symbols[token] = make_pair(pos, 0);
	};
	
	// All labels are relative
	relatives.push_back(pos);
	
	// Set a flag telling that a '+' might come next
	flags[4] = 1;
	
	return token;
}

// Check if it is a number and if flags are raised
bool is_argument(string token, bool *flags, int &pos, string last_label){
	
	// First, make sure there are directives waiting for arguments
	if(flags[1]){
		// SPACE only deals with decimals
		if(is_decimal(token)){
			int value = (int)strtol(token.c_str(), NULL, 10);
			
			if(value>0){
				// Allocates space in the memory
				code.insert(code.end(), value, 0);
				pos+=value;
				
				// Reset flag
				flags[1] = 0;
				return 1;
			}
			else {
				cout << "SPACE can only deal with positive arguments:" << endl;
				return 0;
			};
		}
		else {
			cout << "Could not resolve argument for SPACE directive:" << endl;
			return 0;
		};
	}
	else if(flags[2]){
		// CONST deals with signed, decimal or hexadecimal
		if(is_decimal(token)){
			int value = (int)strtol(token.c_str(), NULL, 10);
			
			// Allocates space in the memory
			code.push_back(value);
			pos++;
			
			// Reset flag
			flags[2] = 0;
			return 1;
		}
		else if(is_hexadecimal(token)){
			int value = (int)strtol(token.c_str(), NULL, 16);
			
			// Allocates space in the memory
			code.push_back(value);
			pos++;
			
			// Reset flag
			flags[2] = 0;
			return 1;			
		}
		else {
			cout << "Could not resolve argument for SPACE directive:" << endl;
			return 0;
		};			
	}
	else if(flags[4]){
		// Expecting a '+' sign
		if(token == "+"){
			// Set a flag that tells if a number is found next, it represent an offset for a label
			flags[5] = 1;
			
			// Reset flag
			flags[4] = 0;
			
			return 1;
		};	
	}
	else if(flags[5]){
		// Expecting a number
		if(is_decimal(token)){
			int value = (int)strtol(token.c_str(), NULL, 10);
			
			// Checks if the label was already defined
			short int ver = already_defined(last_label); 
			
			if(ver == 1){
				// Means that the label was already defined, simply sum in the code the offset
				code[pos-1] += value;
			}
			else {
				// Inserts into the offset maping structure that the position needs an offset
			
				// pos-1 because a label increments the position 
				offsets.insert( pair<int, int>(pos-1, value) );
			
			};

			// Reset flag
			flags[5] = 0;

			return 1;
		};
	}
	
	return 0;
}

// This function creates a sequence of the numerical code that represents the source code
void assemble(ifstream &source){
	string token, last_instruction="", last_label="";
	int pos = 0;
	
	// 0 -> flag_end
	// 1 -> waiting_argument_SPACE
	// 2 -> waiting_argument_CONST
	// 3 ->	waiting_def_arg
	// 4 -> '+' sign expected for label
	// 5 -> waiting for argument for '+' sign
	bool flags[6];
	
	// Initialization of flags
	flags[0] = 0;	// Code was not finished yet
	flags[1] = 0;	// Not waiting for SPACE argument
	flags[2] = 0;	// Not waiting for CONST argument
	flags[3] = 0;	// Not waiting for definiton arguments (instruction or directive SPACE or CONST)
	flags[4] = 0;	// Does not accept '+' sign
	flags[5] = 0;	// No argument for '+'
	
	short int cnt = 0;	// cnt is only 0, 1 or 2, depends on the instruction

	while(!source.eof()){
		// Get next token
		token = get_token(source);
		
		// If there is a token
		if(!token.empty()){
			// Check if token is a instruction, label definition or none (assume label)
			if(is_instruction(token)){

				// Check if there are problems according to flags
				if(check_problems(cnt, flags, last_instruction, pos, 0, token) != 2){
					// This function knows what to do when an instruction is found
					last_instruction = solve_instruction(token, flags, cnt);
					// Increment position of the next memory block
					pos++;					
				};
			}
			else if(is_directive(token)){
			
				// Check if there are problems according to flags
				check_problems(cnt, flags, last_instruction, pos, 1, token);
				
				// Deals with the directives
				solve_directive(token, flags);
			}
			else if(is_label_def(token)){
				// If is label definition, the checking function already erases the ':'
	
				if(valid(token, cnt)){
					// Token is valid

					// Check if there are problems according to flags				
					check_problems(cnt, flags, last_instruction, pos, 2, token);
				
					solve_label_def(token, flags, pos);
				}
				else {
					cout << "Invalid label definition: \"" << token << "\"!" << endl;
				};
			}
			else {
				// Assume it is a label
				if(valid(token, cnt)){
					// Token is valid
	
					// Check most problems
					if(!check_problems(cnt, flags, last_instruction, pos, 3, token)){
						last_label = solve_label(token, pos, flags);
						cnt--;
						pos++;
					};
				}
				else {
					if(!is_argument(token, flags, pos, last_label)){
						cout << "Invalid token: \"" << token << "\"!" << endl;
					};
				};
			};
		};
		
		// cout << "Token: " << token << endl;
		// cout << "Flags: end: " << flags[0] << ", space: " << flags[1] << ", const: " << flags[2] << ", def: " << flags[3] << ", +: " << flags[4] << ", +arg: " << flags[5] << endl;
		
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
