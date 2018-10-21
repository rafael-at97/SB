def: equ 0
mod: BEGIN
SECTION TEXT
	INPUT N1
	INPUT N2
	if def
	DIV N4
	ADD N2
	STORE N3
	OUTPUT N3
	STOP
SECTION BSS
N1:	SPACE 
N2:	SPACE
N3:	SPACE
SECTION DATA
N4: CONST 2
END	
	
; SPACE or CONST alone in a line is a problem, or label
; Have to take care after there file eof, if the assembler is waiting for something
; Check the size allocated for a label? Example: N: SPACE 2 but allows ADD N + 3
; Remember to check all flags after exit main loop
