all: montador ligador clean move

ligador: ligador.o files.o scanner.o
	g++ -Wall -ansi -std=c++98 ligador.o files.o scanner.o -o ligador

ligador.o: ligador.cpp
	g++ -Wall -ansi -std=c++98 ligador.cpp -c -o ligador.o

montador: montador.o files.o scanner.o
	g++ -Wall -ansi -std=c++98 montador.o files.o scanner.o -o montador	

montador.o: montador.cpp
	g++ -Wall -ansi -std=c++98 montador.cpp -c -o montador.o

files.o: files.cpp files.h
	g++ -Wall -ansi -std=c++98 files.cpp -c -o files.o
	
scanner.o: scanner.cpp scanner.h
	g++ -Wall -ansi -std=c++98 scanner.cpp -c -o scanner.o	

.PHONY: clean

clean:		
	rm -f *.o

move:
	mkdir -p ./Simuladores
	mv -f ligador ./Simuladores
	mv -f montador ./Simuladores
