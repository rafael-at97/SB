montador: main.o files.o scanner.o
	g++ -Wall -ansi main.o files.o scanner.o -o montador	

main.o: trab1.cpp
	g++ -Wall -ansi trab1.cpp -c -o main.o

files.o: files.cpp files.h
	g++ -Wall -ansi files.cpp -c -o files.o
	
scanner.o: scanner.cpp scanner.h
	g++ -Wall -ansi scanner.cpp -c -o scanner.o	

.PHONY: clean

clean:		
	rm -f *.o
