CC=g++
STD=-std=c++11
CF=$(STD) 
LINK_FLAGS = -pthread


CSVParser.out: CSVParser.o
	$(CC) $(CF) $(LINK_FLAGS) -o CSVParser.out CSVParser.o

main.o: main.cpp
	$(CC) $(CF) $(LINK_FLAGS) -c CSVParser.cpp -o CSVParser.o


.PHONY: clean
clean:
	rm -rf *.o *.out