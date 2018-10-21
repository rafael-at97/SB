SECTION Text
	COPY N1, N2
Rot:ADD N1 + 1	
SECTION BSS
N1: SPACE 3
SECTION DATA
N2: CONST +5

; SPACE or CONST alone in a line is a problem, or label
; Have to take care after there file eof, if the assembler is waiting for something
; Check the size allocated for a label? Example: N: SPACE 2 but allows ADD N + 3
; Remember to check all flags after exit main loop
