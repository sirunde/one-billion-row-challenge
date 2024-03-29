CC=g++
TARGET=main

all: $(TARGET)

run:
	./main input.txt output.txt
	
$(TARGET): $(TARGET).cpp
	$(CC) -pthread -g -Wall -Wextra mmain.cpp -o main -O3

clean:
	rm -rf *.o $(TARGET)
