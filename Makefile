CC=g++

all: main

run:
	./main input.txt output.txt
	
main: main.o
	$(CC) -o $@ main.o

clean:
	rm -rf *.o