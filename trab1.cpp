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
 */ 

#include<cstdlib>
#include<iostream>
#include<map>
#include<string>
#include<fstream>
#include<vector>
#include"files.h"
#include"scanner.h"

using namespace std;

// Table of instructions, ordered to make it easier to find mnemonics
map<string, int> instructions;

// Table of symbols
map<string, int> symbols;

// Vectors para codigos (opcode e espacos na memoria) e informacao de codigos relativos
vector<int> code;
vector<int> relatives;

void fill_instruction_table(){
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

bool is_rotule_def(string &token){
	if(token[token.length() - 1] == ':'){
		token = token.substr(0, token.length()-1);
		return 1;
	};
	return 0;
}

// This function creates a sequence of the numerical code that represents the source code
void assemble(ifstream &source){
	string token;

	while(!source.eof()){
		// Get next token
		token = get_token(source);
		
		// If there is a token
		if(!token.empty()){
			// Check if token is a instruction, rotule definition or none (assume rotule)
			if(is_instruction(token)){
				cout << "Instruction: " << token << endl;
			}
			else if(is_rotule_def(token)){
				// If is rotule definition, the checking function already erases the ':'
				cout << "Rotule definition: " << token << endl;
			}
			else {
				// Assume it is a rotule
				if(valid(token)){
					// Token is valid
					cout << "Valid token: " << token << endl;
				}
				else {
					cout << "Invalid token: " << token << endl;
				};
			};
		};
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
	fill_instruction_table();

	// Function that reads the input file and fills the 'code' and 'relative' vectors, most important function in the code
	assemble(source);

	// Print the results as expecified
	print_code();
		
	//cout << instructions["ADD"] << endl;
	return 0;
}
