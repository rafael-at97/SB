def: equ 10
SECTION TEXT
	INPUT N1
	INPUT N2
	LOAD N1
	ADD N2
	STORE N3
	OUTPUT N3
	STOP
	IF def
SECTION BSS	
N1:	SPACE def
N2:	SPACE
N3:	SPACE	
SECTION DATA
	
; SPACE or CONST alone in a line is a problem, or label
; Have to take care after there file eof, if the assembler is waiting for something
; Check the size allocated for a label? Example: N: SPACE 2 but allows ADD N + 3
; Remember to check all flags after exit main loop
