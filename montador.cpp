/*
 *	Trabalho 01 - Software Basico - Departamento de Ciencia da Computacao - Universidade de Brasilia
 *	Professor:	Bruno Machiavello
 *	Alunos:		Thiago Holanda - Matricula: 13/0018007 
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
 *		V2.5 - Need to detect if a ',' is used between COPY instruction
 *		V2.6 - Errors are now referenced by line number
 *		V3.0 - Accept directives SECTION
 *		V3.1 - Cannot use certain instructions involving constants
 *	->	V4.0 - Final 
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
/*					  Example
 *	| Name | Value  | Defined | Size | section |
 *  |   A  |    2   |    0	  |   4  |    1	   |
 *  |   B  |    1   |    1 	  |   1  |    2	   |
 *
 */
class symbol{
	public:
		symbol(){
			// Constructor
			this->value = -1;
			this->defined = 0;
			this->size = 1;
			this->section = 0;
			this->type = -1;
		};
		symbol(int _value, bool _defined, int _size, short int _section, short int _type){
			// Constructor
			this->value = _value;
			this->defined = _defined;
			this->size = _size;
			this->section = _section;
			this->type = _type;
		};		
		int value;
		bool defined;
		int size;
		short int section;	// 0->TEXT, 1->DATA, 2->BSS
		short int type; // -1->Undefined, 0->Local, 1->EXTERN 
		vector<pair<int, int> > access;
		vector<int> ext;	// Extern access
};

map<string, symbol > symbols;

// Table that helps deal with offsets
map<int, pair<int, int> > offsets;

// Table used for pre-processing
map<string, int> pre_proc;

// Directives table
set<string> directives;

// Definitions table (PUBLIC)
set<string> definitions;

// Vectors to code (opcode and memory spaces) and information on relative positions
vector<int> code;
vector<int> relatives;

void fill_instruction_and_directives_table(){
	// Insertion of instructions in the table
	instructions.insert( pair<string, int>("ADD", 1) );
	instructions.insert( pair<string, int>("SUB", 2) );
	instructions.insert( pair<string, int>("MULT", 3) );
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
	directives.insert("SECTION");
	directives.insert("TEXT");
	directives.insert("DATA");
	directives.insert("BSS");
	directives.insert("BEGIN");
	directives.insert("END");
	directives.insert("PUBLIC");
	directives.insert("EXTERN");
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
	map<string, symbol >::iterator symbol_it;
	
	symbol_it = symbols.find(token);

	if(symbol_it != symbols.end()){
		// The token is a instruction
		
		// Return 0 or 1, depends if it was already defined
		return (symbol_it->second).defined;
	}
	else
		return -1;
}

// Check if a position is in the offsets mapping data structure and returns the value of the offset
int check_offset(int pos, int &line_number){
	map<int, pair<int, int> >::iterator it;
	
	it = offsets.find(pos);
	
	if(it!=offsets.end()){
		// If a result was found
		int value = (it->second).first;
		line_number = (it->second).second;
		
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
	int _pos, tmp, line_number;
	
	// Save the position pointed by the symbols table
	_pos = symbols[token].value;
	
	do {
		// Avoid losing reference to the position pointed in the code
		tmp = code[_pos];
		
		// Update the position in the code
		code[_pos] = pos;
		
		// Check if it needs offsets
		int value = check_offset(_pos, line_number);
		
		// Check if offset is within size allocated for label
		if(value < symbols[token].size){
			// Okay
			code[_pos] += value;
		}
		else {
			// Not okay
			cout << "Error on line " << line_number << ": Segmentation Fault! (Erro semântico)" << endl;
		};
		
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

// Check if public already defined
bool public_defined(string token){
	set<string>::iterator it;
	it = definitions.find(token);
	
	if(it!=definitions.end())
		return 1;
		
	return 0;	
}

// Add extern to symbols table
void add_extern(string label, int line_number){
	// Check if it is not a public label
	if(public_defined(label)){
		cout << "Error on line " << line_number << ": \"" << label << "\" already defined as PUBLIC! (Erro semântico)" << endl;
		return;
	}
	
	// The solve_label_def will have already inserted the label into the symbols table, we just need to update it
	if(symbols[label].type == -1){
		// A label of this was already seen
		cout << "Error on line " << line_number << ": Can not use EXTERN variables before definition! (Erro semântico)" << endl;
	}
	else {
		// Update symbols table with value 0, defined, size=-1(Unknow), section = -1 (Can do everything), type = 1
		symbols[label].value = 0;
		symbols[label].defined = 1;
		symbols[label].size = -1;
		symbols[label].section = -1;
		symbols[label].type = 1;
	};
}

// Until now, returns nothing
void solve_directive(string token, bool *flags, short int &sections, short int &module, string last_label, int line_number){
	// First, make sure token is uppercase
	token = upper(token);				
					
	if(flags[6] == 1){
		// If expecting TEXT, DATA or BSS
		if(token == "TEXT"){
			if(!sections){
				// If actually expecting TEXT
				
				// Tells the assembler it is now in the section text area
				sections = 1;
			}
			else {
				cout << "Error on line "<< line_number << ": Redefinition of SECTION TEXT! (Erro semântico)" << endl;
			};
		}
		else if(token == "DATA"){
			if(!sections){
				cout << "Error on line "<< line_number << ": SECTION DATA cannot be defined before SECTION TEXT! (Erro semântico)" << endl;
			}
			else if(sections == 1){
				// DATA after TEXT
				sections = 2;
			}
			else if(sections == 4){
				// DATA after BSS
				sections = 3;
			}
			else {
				cout << "Error on line "<< line_number << ": Redefinition of SECTION DATA! (Erro semântico)" << endl;
			};
		}
		else if(token == "BSS"){
			if(!sections){
				cout << "Error on line "<< line_number << ": SECTION BSS cannot be defined before SECTION TEXT! (Erro semântico)" << endl;
			}
			else if(sections == 1){
				// BSS after TEXT
				sections = 4;
			}
			else if(sections == 2){
				// BSS after DATA
				sections = 5;
			}
			else {
				cout << "Error on line "<< line_number << ": Redefinition of SECTION BSS! (Erro semântico)" << endl;
			};		
		}
		else {
			// Not what was expected, error!
			cout << "Error on line "<< line_number << ": Could not resolve section name! (Erro sintático)" << endl;
		};
		
		// Reset flag
		flags[6] = 0;
	};
		
	if(flags[3] == 1){
		// Label definition waiting for SPACE or CONST
		// Specific cases, SPACE and CONST				
		if(token == "SPACE"){
			// Check a flag and tells the assembler it is waiting for an argument
			flags[1] = 1;
		}
		else if(token == "CONST"){
			// Check a flag and tells the assembler it is waiting for an argument
			flags[2] = 1;
		}
		else if(token == "BEGIN"){
			// Begin module, will be solved ahead, just update value of the symbol in the table
			symbols[last_label].value = 0;
			symbols[last_label].size = 0;
			symbols[last_label].section = -1;
			symbols[last_label].defined = 1;
			symbols[last_label].type = 0;	// Purely local
		}
		else if(token == "EXTERN"){
			// Extern variable
			add_extern(last_label, line_number);
		}
		else {
			cout << "Error on line "<< line_number << ": Can not start label with this directive! (Erro sintático)" << endl;
		};
		
		// Reset flag
		flags[3] = 0;
	}
	else {
		if(token == "SPACE" || token == "CONST"){
			cout << "Error on line "<< line_number << ": A label is needed to define SPACE or CONST directives! (Erro sintático)" << endl;
		};
	}
					
	if(token == "SECTION"){
		// Set a flag telling the assembler it wants TEXT, DATA or BSS
		flags[6] = 1;
	}
	else if(token == "BEGIN"){
		// Informing it is a module
		
		if(sections!=0){
			// Comes before SECTION TEXT
			cout << "Error on line "<< line_number << ": Module can not begin here! (Erro semântico)" << endl;
		}
		else {
			if(module == 0){
				module = 1;
			}
			else {
				// Begin already detected
				cout << "Error on line "<< line_number << ": BEGIN was already detected! (Erro semântico)" << endl;
			};
		};
	}
	else if(token == "END"){
		if(module == 1){
			module = 2;
		}
		else {
			// END without BEGIN or END already detected
			if(module == 0)
				cout << "Error on line "<< line_number << ": Cannot END module without BEGIN first! (Erro semântico)" << endl;
			else {
				// Never expecting to come
				cout << "Error on line "<< line_number << ": END already detected! (Erro semântico)" << endl;
			};
		};		
	}
	else if(token == "PUBLIC"){
		// Set a flag that tells next label is to be made public
		flags[7] = 1;
	};
}

// Define the labels
void define_label(string label, int pos, int value, int _size, short int _section, short int _type){
	// solve_label_def will make sure the symbols table already contains the label
	
	// Update values in the table
	symbols[label].size = _size;
	symbols[label].section = _section;
	symbols[label].type = _type;
	
	if(symbols[label].defined == 0){
		recursive_definition(label, pos);
		symbols[label].value = pos;
		symbols[label].defined = 1;
	};
	
}

// Check most flags and solve some problems 0: No error, 1: Argument errors, 2: STOP error
short int check_problems(short int &cnt, bool *flags, string last_instruction, int &pos, short int type, string token, int line_number, string last_label){

	// If is an instruction but STOP was already found
	if(type == 0 && flags[0]){
		cout << "Error on line "<< line_number << ": Stop already detected, cannot handle any more instructions!! (Erro semântico)" << endl;
		return 2;
	};

	if(flags[1]){
		// Was expecting an argument for SPACE
		// If not found, simply puts a 0 in the code and increment memory position
		code.push_back(0);
		
		define_label(last_label, pos, 0, 1, 2, 0);
		pos++;
		
		// Reset flag
		flags[1] = 0;
		
		// Waiting for SPACE arguments and not finding any is not an error
		// return 1;
	};
	
	// If cnt is different from 0, means that an argument label was expected, print error and reset cnt
	if(cnt && type!=3){
		cout << "Error on line " << line_number << ": Missing arguments for instruction " << last_instruction << "! (Erro sintático)" << endl;
		
		// Reset counter so the next instruction can update the counter accordingly
		cnt = 0;
		return 1;
	}
	else if(!cnt && type == 3 && flags[7]!=1){
		cout << "Error on line " << line_number << ": Too many arguments for instruction " << last_instruction << "! (Erro sintático)" << endl;
		return 1;
	}
	
	if(flags[2]){
		// Was expecting an argument for CONST directive
		// If not found, print error
		cout << "Error on line " << line_number << ": Expected argument for \"CONST\" directive! (Erro sintático)" << endl;
		
		// Does not increment 'pos' because the const was not declared correctly
		
		// Reset flag because error was already printed
		flags[2] = 0;
		return 1;
	};
	if(flags[3]){
		// Was expecting instruction or directives (SPACE and CONST only)
		
		if(type!=0 && type!=1){
			return 1;
		}
		else{
			if(type == 0){
				// Reset flag only if instruction found
				flags[3] = 0;
				
				// Need to update the symbol table, informing the last label def was a instruction
				define_label(last_label, pos, 0, 1, 0, 0);
			};	
		};
	};
	if(flags[4]){
		// Plus sign was expected, not found, simply reset flag
		flags[4] = 0;
	};
	if(flags[5]){
		// Expecting argument for '+' sign, not found means error
		cout << "Error on line " << line_number << ": Missing argument for '+' operand! (Erro sintático)" << endl;
		
		// Reset flag
		flags[5] = 0;
		return 1;
	};
	if(flags[6]){
		// If waiting for directive TEXT, DATA or BSS
		if(type != 1){
			cout << "Error on line " << line_number << ": Expecting directive! (Erro sintático)" << endl;
		
			// Reset flag
			flags[6] = 0;
		
			return 1;	// If was expecting but found anything other that a directive, error	
		};
	};
	if(flags[7]){
		if(type!=3){
			cout << "Error on line "<< line_number << ": Wrong argument for PUBLIC directive! (Erro sintático)" << endl;
			return 1;
		};
	};
	
	// No problems found
	return 0;
}

// Solve labels definition
string solve_label_def(string token, bool *flags, int pos, int line_number){
	token = upper(token);
	
	// 1: Exists and defined, 0: Exists but not defined, -1: Does not exist
	short int ver = already_defined(token);
	
	// Already defined
	if(ver == 1){
		cout << "Error on line " << line_number << "! Redefinition of label! (Erro semântico)" << endl;
	}
	else if(!ver){
		// Already exists, but not defined
		// Tell assembler it is expecting an instruction or some directives (SPACE and CONST or EXTERN)
		flags[3] = 1;
	}
	else {
		// -1, does not even exist!
		
		// CHANGE POSITION TO THE COUNTER IN THE CODE, the only values needed here are value and defined
		symbols.insert(make_pair<string, symbol>(token, symbol(pos, 1, 1, 1, 0)));
		
		// Tell assembler it is expecting an instruction or some directives (SPACE and CONST or EXTERN)
		flags[3] = 1;
	};
	
	return token;
}

// Returns 0-Type 'R' instructions, 1-'J' instructions and 2-'S' instructions
int instruction_type(string instruction, short int cnt){
	if(instruction == "STORE" || instruction == "INPUT" || (instruction == "COPY" && cnt == 1)){
		// Trying to change element that is CONST
		// Type 'S' instruction
		return 2;
	}
	else if(instruction.substr(0, 3) == "JMP"){
		// Jump instruction, type 'J'
		return 1;
	}
	else if(instruction == "DIV"){
		// Verify division by 0
		return 3;
	};
	return 0;
}

// Deals with labels
string solve_label(string token, int pos, bool *flags, short int cnt, string instruction, int line_number){
	token = upper(token);

	// 1: Exists and defined, 0: Exists but not defined, -1: Does not exist
	short int ver = already_defined(token); 

	if(ver == 1){
		// Already exists and defined, simply insert into the code the equivalent memory location
		code.push_back(symbols[token].value);
		
		// If it is extern, the value 0 will go to the code, need to add into the symbol vector the position being used
		if(symbols[token].type == 1)
			symbols[token].ext.push_back(pos);
		
		// Also inserts into the symbols vector the operation
		symbols[token].access.push_back(pair<int, int>(instruction_type(instruction, cnt), line_number));
	}
	else if(!ver){
		// Isn't defined yet
		code.push_back(symbols[token].value);
		symbols[token].value = pos;
		symbols[token].access.push_back(pair<int, int>(instruction_type(instruction, cnt), line_number));
	}
	else {
		// First time seeing label
		code.push_back(-1);
		symbols.insert(make_pair<string, symbol>(token, symbol(pos, 0, 1, 1, -1)));
		symbols[token].access.push_back(pair<int, int>(instruction_type(instruction, cnt), line_number));
	};
	
	// All labels are relative
	relatives.push_back(pos);
	
	// Set a flag telling that a '+' might come next
	flags[4] = 1;
	
	return token;
}

// Check if it is a number and if flags are raised
bool is_argument(string token, bool *flags, int &pos, string last_label, short int cnt, int line_number){
	
	// First, make sure there are directives waiting for arguments
	if(flags[1]){
		// SPACE only deals with decimals
		if(is_decimal(token, cnt)){
			int value = (int)strtol(token.c_str(), NULL, 10);
			
			if(value>0){
				// Allocates space in the memory
				code.insert(code.end(), value, 0);
				define_label(last_label, pos, 0, value, 2, 0);
				
				pos+=value;
				
				// Reset flag
				flags[1] = 0;
				return 1;
			}
			else {
				cout << "Error on line "<< line_number << ": SPACE can only deal with positive arguments! (Erro sintático)" << endl;
				return 0;
			};
		}
		else {
			cout << "Error on line "<< line_number << ": Could not resolve argument for SPACE directive! (Erro sintático)" << endl;
			return 0;
		};
	}
	else if(flags[2]){
		// CONST deals with signed, decimal or hexadecimal
		if(is_decimal(token, cnt)){
			int value = (int)strtol(token.c_str(), NULL, 10);
			
			// Allocates space in the memory
			code.push_back(value);
			define_label(last_label, pos, value, 1, 1, 0);
			pos++;
		
			// Reset flag
			flags[2] = 0;
			return 1;
		}
		else if(is_hexadecimal(token)){
			int value = (int)strtol(token.c_str(), NULL, 16);
			
			// Allocates space in the memory
			code.push_back(value);
			define_label(last_label, pos, value, 1, 1, 0);
			pos++;
			
			// Reset flag
			flags[2] = 0;
			return 1;			
		}
		else {
			cout << "Error on line "<< line_number << ": Could not resolve argument for CONST directive! (Erro sintático)" << endl;
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
		
		// Reset flag
		flags[4] = 0;
	}
	else if(flags[5]){
		// Expecting a number
		if(is_decimal(token, cnt)){
			int value = (int)strtol(token.c_str(), NULL, 10);
			
			// Checks if the label was already defined
			short int ver = already_defined(last_label); 
			
			if(ver == 1){
				// Means that the label was already defined
				
				// Check if the offset is within the declared label
				if((value < symbols[last_label].size) || (symbols[last_label].size == -1)){
					// Okay, simply sum in the code the offset
					code[pos-1] += value;
				}
				else {
					cout << "Error on line "<< line_number << ": Segmentation Fault! (Erro semântico)" << endl;
				};
			}
			else {
				// Inserts into the offset maping structure that the position needs an offset
			
				// pos-1 because a label increments the position 
				offsets.insert( make_pair<int, pair<int, int> >(pos-1, pair<int, int>(value, line_number)));
			
			};

			// Reset flag
			flags[5] = 0;

			return 1;
		};
		// Reset flag
		flags[5] = 0;
	}
	
	return 0;
}

// Print errors related to sections
bool correct_section(string token, short int sections, short int type, int line_number){
	if(type == 0 || type == 2){
		if(sections != 1){
			if(type == 0){
				// Instruction not on text section
				cout << "Error on line "<< line_number << ": Instruction outside TEXT section! (Erro semântico)" << endl;
			}
			else {
				// Label not on text section
				cout << "Error on line "<< line_number << ": Using label outside TEXT section! (Erro semântico)" << endl;			
			}	
			return 0;
		};
		return 1;
	}
	else if(type == 1){
		// Directives
		if(token == "SPACE"){
			// Only accepted in section BSS
			if(sections == 4 || sections == 5){
				return 1;
			}
			else {
				cout << "Error on line "<< line_number << ": SPACE declaration outside BSS section! (Erro semântico)" << endl;
				return 0;
			};
		}
		else if(token == "CONST"){
			// Only accepted in section DATA
			if(sections == 2 || sections == 3){
				return 1;
			}
			else {
				cout << "Error on line "<< line_number << ": CONST declaration outside DATA section! (Erro semântico)" << endl;
				return 0;
			};
		}
		else {
			// Assuming other directives can come in any place of the code
			return 1;
		}
	};
	return 1;
}

void missing_declarations(){
	map<string, symbol >::iterator it;
	set<string>::iterator s_it;
	
	// First, check for undeclared labels
	for(it=symbols.begin();it!=symbols.end();it++){
		if((it->second).defined!=1){
			cout << "\"" << it->first << "\" was not defined anywhere!" << endl;
		};
	};
	
	// Now, check for publics without declaration
	for(s_it=definitions.begin();s_it!=definitions.end();s_it++){
		it = symbols.find(*s_it);
		if(it == symbols.end()){
			cout << "\"" << *s_it << "\" declared public but not defined!" << endl;
		}
		else if((it->second).defined!=1){
			cout << "\"" << *s_it << "\" declared public but not defined!" << endl;
		};
	};
}

void check_instruction_errors(){
	map<string, symbol >::iterator it;
	vector<pair<int, int> >::iterator v_it;
	short int section;
	
	for(it=symbols.begin();it!=symbols.end();it++){
		// Extern symbols can be operands for any kind of instruction
		if((it->second).type == 1){
			continue;
		};
		section = (it->second).section;
		for(v_it=(it->second).access.begin();v_it!=(it->second).access.end();v_it++){
			if(section == 0){
				// Text section label
				if(v_it->first != 1){
					// Different from jump
					cout << "Error on line " << v_it->second << ": cannot operate on label " << it->first << " (Erro semântico)" << endl;
				}
			}
			else if(section == 1){
				// Data section label
				if(v_it->first == 3) {
					if((it->second).value == 0){
						cout << "Error on line " << v_it->second << ": division by 0 detected! (Erro sintático)" << endl;
					};					
				}
				else if(v_it->first != 0){
					// Data labels can only be used as arithmetic operands
					cout << "Error on line " << v_it->second << ": cannot operate on label " << it->first << " (Erro semântico)" << endl;
				}
			}
			else if(section == -1){
				cout << "Error on line " << v_it->second << ": No operation can be done on label " << it->first << " (Erro semântico)" << endl;
			}
			else {
				// BSS section label
				if(v_it->first == 1){
					// BSS section does not support jumps
					cout << "Error on line " << v_it->second << ": cannot operate on label " << it->first << " (Erro semântico)" << endl;
				};
			};
		};
	};
}

// If after the main loop any flag was still raised, deal with it
void fix_final_flags(bool *flags, string last_label, int pos, short int cnt, string instruction, short int module, int line_number){
	if(cnt){
		cout << "Error on line " << line_number << ": Missing argument(s) for instruction " << instruction << "! (Erro sintático)" << endl;
	};
	
	if(module == 1){
		cout << "Error on line " << line_number << ": Missing END of module! (Erro semântico)" << endl;
	};
	
	// Possible flags problem:
	if(!flags[0]){
		// STOP not found
		cout << "Error on line " << line_number << ": STOP instruction not detected! (Erro semântico)" << endl;
	};
	if(flags[1]){
		// Waiting for SPACE argument, not found, insert 0 in the code and update last label
		// If not found, simply puts a 0 in the code and increment memory position
		code.push_back(0);

		// Update label		
		define_label(last_label, pos, 0, 1, 2, 0);
		
		// Reset flag
		flags[1] = 0;
	};
	if(flags[2]){
		// Waiting for CONST argument
		// If not found, print error
		cout << "Error on line " << line_number << ": Expected argument for \"CONST\" directive! (Erro sintático)" << endl;
		
		// Reset flag because error was already printed
		flags[2] = 0;	
	};
	if(flags[3]){
		// Waiting for definition of label
		cout << "Error on line " << line_number << ": Empty label! (Erro sintático)" << endl;
	};
	if(flags[4]){
		// If this is an error, a whole bunch of other errors were already printed
		cout << "Error on line " << line_number << ": TEXT section ending abruptly! (Erro semântico)" << endl;
	};
	if(flags[5]){
		// Same as above
		cout << "Error on line " << line_number << ": Expected argument for operand '+'! (Erro sintático)" << endl;
	};
	if(flags[6]){
		// SECTION without name
		cout << "Error on line " << line_number << ": Unnamed SECTION! (Erro sintático)" << endl;
	};
}

// Check if a symbol is extern
bool is_extern(string token){
	map<string, symbol >::iterator it;
	
	it = symbols.find(token);
	if(it!=symbols.end()){
		if((it->second).type == 1){
			return 1;
		}
		return 0;
	}
	return 0;
}


// Insert into the SET of public labels
void insert_public(string label, int line_number){
	// Check into the definition table if it already exists
	label = upper(label);
	
	if(public_defined(label)){
		cout << "Error on line " << line_number << ": \"" << label << "\" already made public! (Erro semântico)" << endl;
	}
	else {
		// Check into the symbols table if it exists and is EXTERN
		if(is_extern(label)){
			cout << "Error on line " << line_number << ": \"" << label << "\" already defined as EXTERN! (Erro semântico)" << endl;
		}
		else {
			definitions.insert(label);
		};
	};
}

// Function for debugging
void print_symbols(){
	map<string, symbol >::iterator it;
	vector<pair<int, int> >::iterator v_it;
	vector<int>::iterator s_it;
	set<string>::iterator str_it;
	
	cout << "Symbols:" << endl;
	
	for(it=symbols.begin();it!=symbols.end();it++){
		cout << "Symbol: " << (it->first) << ", value: " << (it->second).value << ", defined: " << (it->second).defined;
		cout << ", size: " << (it->second).size << ", section: " << (it->second).section << ", type: " << (it->second).type << endl;
		cout << "USES: " << endl;
		for(v_it=(it->second).access.begin();v_it!=(it->second).access.end();v_it++){
			cout << "Instruction type " << v_it->first << " on line " << v_it->second << endl;
		};
		cout << endl;
		cout << "EXT USES: " << endl;
		for(s_it=(it->second).ext.begin();s_it!=(it->second).ext.end();s_it++){
			cout << "Position: " << *s_it << endl;
		};		
		cout << endl;
	}
	
	cout << "Publics:" << endl;
	
	for(str_it=definitions.begin();str_it!=definitions.end();str_it++){
		cout << *str_it << endl;
	};
	
}

bool check_module_local(){
	// Check if public is empty
	if(!definitions.empty()){
		return 0;
	};
	
	map<string, symbol>::iterator it;
	
	for(it=symbols.begin();it!=symbols.end();it++){
		if((it->second).type == 1){
			// Extern detected
			return 0;
		};
	};
	
	return 1;
}

// Actually saves the code into a file
void save_code_into_file(bool tables, string name){
	int i;
	
	ofstream output_file;
	
	// Change name of file to be written
	name = name.substr(0, name.find('.'));
	name += ".obj";
	
	open_write(output_file, name);

	if(tables == 1){
		// Save table of uses, definition and relatives
		map<string, symbol >::iterator it;
		vector<int>::iterator v_it;
		set<string>::iterator s_it;
	
		// Table os uses
		write(output_file, "TABLE USE");
		write_newl(output_file);
	
		for(it=symbols.begin();it!=symbols.end();it++){
			// Extern symbols can be operands for any kind of instruction
			if((it->second).type == 1){
				for(v_it=(it->second).ext.begin();v_it!=(it->second).ext.end();v_it++){
					write(output_file, it->first + " " + to_string(*v_it));
					write_newl(output_file);
				};
			};
		};
	
		write_newl(output_file);
		
		// Table of definitions
		write(output_file, "TABLE DEFINITION");
		write_newl(output_file);
		
		for(s_it=definitions.begin();s_it!=definitions.end();s_it++){
			// Extern symbols can be operands for any kind of instruction
			write(output_file, *s_it + " " + to_string(symbols[*s_it].value));
			write_newl(output_file);
		};		
		
		write_newl(output_file);
		
		// table of relatives;
		write(output_file, "RELATIVE");
		write_newl(output_file);
		for(i=0;i<(int)relatives.size();i++)
			if(!i)
				write(output_file, to_string(relatives[i]));
			else
				write(output_file, ' ' + to_string(relatives[i]));	
		
		write_newl(output_file);
		write_newl(output_file);
		write(output_file, "CODE");
		write_newl(output_file);
	};

	// Code per se

	for(i=0;i<(int)code.size();i++)
		if(!i)
			write(output_file, to_string(code[i]));
		else
			write(output_file, ' ' + to_string(code[i]));	
		
	write_newl(output_file);

	// Close pre-processed file
	output_file.close();

}

// Save the module
void print_module(short int module, string name){
	if(module == 0){
		// There is supposed to be only local variables, check if there are any public or extern
		if(check_module_local()){
			save_code_into_file(0, name);
		}
		else {
			cout << "Cannot use EXTERN or PUBLIC directives in a single module! (Erro semântico)" << endl;
		};
	}
	else {
		// It is a module, not a stand-alone
		save_code_into_file(1, name);
	};
}

// This function creates a sequence of the numerical code that represents the source code
void assemble(ifstream &source, string name){
	string token, last_instruction="", last_label="";
	int pos = 0, line_number=1, last_line=0;
	
	short int module = 0; // 0 -> Not module, 1 -> detected BEGIN directive, 2 -> detected END directive
	
	bool break_line = 0; // Needed to maitain correct line_number
	
	// 0 -> flag_end
	// 1 -> waiting_argument_SPACE
	// 2 -> waiting_argument_CONST
	// 3 ->	waiting_def_arg
	// 4 -> '+' sign expected for label
	// 5 -> waiting for argument for '+' sign
	// 6 -> waiting for directives TEXT, BSS or DATA
	// 7 -> Waiting for argument for PUBLIC
	bool flags[8];
	
	// Initialization of flags
	flags[0] = 0;	// Code was not finished yet
	flags[1] = 0;	// Not waiting for SPACE argument
	flags[2] = 0;	// Not waiting for CONST argument
	flags[3] = 0;	// Not waiting for definiton arguments (instruction or directive SPACE or CONST)
	flags[4] = 0;	// Does not accept '+' sign
	flags[5] = 0;	// No argument for '+'
	flags[6] = 0;   // Not waiting yet
	flags[7] = 0;
	
	// 0 -> None found yet
	// 1 -> TEXT
	// 2 -> TEXT then DATA
	// 3 -> TEXT then BSS then DATA
	// 4 -> TEXT then BSS
	// 5 -> TEXT then DATA then BSS
	short int sections = 0;
	
	short int cnt = 0;	// cnt is only 0, 1 or 2, depends on the instruction

	while(!source.eof()){
		// Get next token
		token = get_token(source, line_number, break_line);
		
		// If there is a token
		if(!token.empty()){
		
			// If module already ended
			if(module==2){
				cout << "Error on line " << line_number << ": Ignoring, END already detected! (Erro semântico)" << endl;
				continue;
			};
		
			// Check if token is a instruction, label definition or none (assume label)
			if(is_instruction(token)){

				// Check section problems
				if(correct_section(token, sections, 0, line_number)){
					// Check if there are problems according to flags
					if(check_problems(cnt, flags, last_instruction, pos, 0, token, line_number, last_label) != 2){
						// This function knows what to do when an instruction is found
						last_instruction = solve_instruction(token, flags, cnt);
						// Increment position of the next memory block
						pos++;					
					};
				};
			}
			else if(is_directive(token)){
			
				if(correct_section(token, sections, 1, line_number)){
					// Check if there are problems according to flags
					check_problems(cnt, flags, last_instruction, pos, 1, token, line_number, last_label);
				
					// Deals with the directives
					solve_directive(token, flags, sections, module, last_label, line_number);
				};
			}
			else if(is_label_def(token)){
				// If is label definition, the checking function already erases the ':'
	
				if(valid(token, cnt)){
					// Token is valid

					// Check if there are problems according to flags				
					check_problems(cnt, flags, last_instruction, pos, 2, token, line_number, last_label);
	
					if(last_line == line_number){
						cout << "Error on line " << line_number << ": Cannot declare more than one label in the same line! (Erro sintático)" << endl;
					};		
					last_label = solve_label_def(token, flags, pos, line_number);
				}
				else {
					cout << "Error on line " << line_number << ": Invalid label definition: \"" << token << "\"! (Erro léxico)" << endl;
				};
			}
			else {
				// Assume it is a label
				if(valid(token, cnt)){
					// Token is valid
	
					if(correct_section(token, sections, 2, line_number)){
						// Check most problems
						if(!check_problems(cnt, flags, last_instruction, pos, 3, token, line_number, last_label)){
							if(flags[7] == 1){
								// No need to solve anything, simply inserts into the public table and dont change anything
								insert_public(token, line_number);
								
								// Reset flag
								flags[7] = 0;
							}
							else {
								last_label = solve_label(token, pos, flags, cnt, last_instruction, line_number);
								cnt--;
								pos++;
							};
						};
					};
				}
				else {
					if(!is_argument(token, flags, pos, last_label, cnt, line_number)){
						cout << "Error on line " << line_number << ": Invalid token: \"" << token << "\"! (Erro léxico)" << endl;
					};
				};
			};
		};
		last_line = line_number;
		//cout << "Token: " << token << endl;
		//cout << "Flags: end: " << flags[0] << ", space: " << flags[1] << ", const: " << flags[2] << ", def: " << flags[3] << ", +: " << flags[4] << ", +arg: " << flags[5] << ", section: " << flags[6] << ", section: " << sections << endl;
		
	};
	
	fix_final_flags(flags, last_label, pos, cnt, last_instruction, module, line_number);
	missing_declarations();
	check_instruction_errors();
	print_module(module, name);
	//print_symbols();

}

// Check tables of use and definition
void check_tables(){
	map<string, symbol >::iterator it;
	vector<int>::iterator v_it;
	
	cout << "TABLE USE" << endl;
	
	for(it=symbols.begin();it!=symbols.end();it++){
		// Extern symbols can be operands for any kind of instruction
		if((it->second).type == 1){
			for(v_it=(it->second).ext.begin();v_it!=(it->second).ext.end();v_it++){
				cout << it->first << " " << *v_it << endl;
			};
		};
	};
	
	cout << endl;	
}

void print_code(){
	int i;
	
	check_tables();
	
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

bool EQU_defined(string token){
	token = upper(token);
	
	map<string, int>::iterator m_it;
	
	m_it = pre_proc.find(token);
	
	if(m_it != pre_proc.end()){
		return 1;
	};
	
	return 0;
}

// Write into the pre-processed file and returns its name
string write_new_file(vector<pair<string, int> > file_sequence, string name){
	ofstream output_file;
	
	// Change name of file to be written
	name = name.substr(0, name.find('.'));
	name += ".pre";
	
	open_write(output_file, name);

	vector<pair<string, int> >::iterator it;
	int line = 1;
	
	for(it=file_sequence.begin();it!=file_sequence.end();it++){
		while(it->second != line){
			write_newl(output_file);
			line++;
		};
		write(output_file, (it->first + " "));
	};
	
	// Close pre-processed file
	output_file.close();

	return name;
}

// Does the pre-processing
string pre_process(ifstream &source, string name){
	string token, last_label;
	bool break_line = 0;

	int line_number = 1, line_ignore = 0;
	bool equates = 1; // Assume there are EQU's
	bool if_flag = 0;
	short int order=0;  // Order to EQU operands
	
	vector<pair<string, int> > file_sequence;
	
	while(!source.eof()){
		
		// Get next token
		token = get_token(source, line_number, break_line);

		// If there is a token
		if(!token.empty()){
			// First, check if it still accepts EQU's
			
			if(line_number == line_ignore)
				continue;
			
			if(equates == 1){
				if(order == 0){
					// First, search for label definition
					if(is_label_def(token)){
						// Check if already exists
						// If does -> error
						// If not -> Insert into equ table
						if(!EQU_defined(token)){
							last_label = token;
							token = upper(token);						
							pre_proc.insert(pair<string, int>(token, -1));
							file_sequence.push_back(pair<string, int>(last_label + ":", line_number));
						}
						else {
							cout << "Error on line " << line_number << " during pre-processing: Redefinition of EQU directive. (Erro semântico)" << endl;
							return "";
						};
						
						order = 1;
					}
					else {
						// Does not accept any more EQU
						file_sequence.push_back(pair<string, int>(token, line_number));
						equates = 0;
					};
				}
				else if(order == 1){
					// Waiting for EQU
					if(upper(token) == "EQU"){
						file_sequence.push_back(pair<string, int>(token, line_number));
						order = 2;
					}
					else {
						// Not accepting more EQU, erases from EQU table last label
						
						// Insert into the thing to be written in the file 'last_label:' and 'token'
						file_sequence.push_back(pair<string, int>(token, line_number));
						last_label = upper(last_label);
						pre_proc.erase(last_label);
						
						equates = 0;
					};
				}
				else {
					// Waiting for number argument
					if(is_decimal(token, 0)){
						// Update into table the value of the EQU
						last_label = upper(last_label);
						pre_proc[last_label] = (int)strtol(token.c_str(), NULL, 10);
						// Reset order so accepts more EQU's
						// Erases from the file_sequence last two entries, meaning the label definition and the 'EQU'
						file_sequence.pop_back();
						file_sequence.pop_back();
						
						order = 0;
					}
					else {
						// Not accepting more EQU, erases from EQU table last label
						
						// Insert into the thing to be written in the file 'last_label:', 'EQU' and 'token'
						file_sequence.push_back(pair<string, int>(token, line_number));
						last_label = upper(last_label);
						pre_proc.erase(last_label);
						
						cout << "Error on line " << line_number << ": Wrong argument for EQU directive! (Erro sintático)" << endl;
						return "";
						
						equates = 0;
					};
				};
			}
			else {
				// Not accepting more EQU's, treat IF's
				if(if_flag == 1){
					if(EQU_defined(token)){
						// If exists
						token = upper(token);
						int value = pre_proc[token];
						if(!value){
							// Ignore next line, write empty lines to maintain counter
							line_ignore = line_number+1;						
						};
					}
					else {
						cout << "Error on line " << line_number << ": Could not resolve argument for IF instruction! (Erro sintático)" << endl;
						return "";
					};
					// Reset flag
					if_flag = 0;
				}
				else {
					if(is_label_def(token)){
						// Check redefinition of something that was already used as EQU directive
						if(EQU_defined(token)){
							cout << "Error on line " << line_number << ": Label already used as EQU label! (Erro semântico)" << endl;
							return "";
						}
						else {
							// Copy the token
							file_sequence.push_back(pair<string, int>((token+':'), line_number));
						};
					}
					else if(EQU_defined(token)){
						// Instead of writing 'token' to the file, write the respective value of the label
						token = upper(token);
						file_sequence.push_back(pair<string, int>(to_string(pre_proc[token]), line_number));
					}
					else if(upper(token) == "IF"){
						// Deal with IF directive
						// Set a flag waiting for label
						if_flag = 1;
					}
					else if(upper(token) == "EQU"){
						cout << "Error on line " << line_number << ": EQU directive outside proper place! (Erro semântico)" << endl;
						return "";
					}
					else {
						file_sequence.push_back(pair<string, int>(token, line_number));
					};
				};
			};
		};
	};
	
	return write_new_file(file_sequence, name);
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
	if(!open_and_check(source, string(argv[1]), ".asm"))
		return 0;

	// Fill the instruction table for future queries
	fill_instruction_and_directives_table();

	// Pre-processing
	string new_source = pre_process(source, string(argv[1]));

	// Close source file
	source.close();

	// Associates source now with preprocessed file
	if(!open_pre(source, new_source))
		return 0;

	// Function that reads the input file and fills the 'code' and 'relative' vectors, most important function in the code
	assemble(source, new_source);

	// Close the pre-processed file	
	source.close();

	// Print the results as expecified
	//print_code();
		
	//cout << instructions["ADD"] << endl;
	return 0;
}
