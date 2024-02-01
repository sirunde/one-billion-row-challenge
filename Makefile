CC=g++
TARGET=mmain

all: $(TARGET)

run:
	./main input.txt output.txt
	
$(TARGET): $(TARGET).cpp
	$(CC) -g -Wall -Wextra mmain.cpp -o main -O2

clean:
	rm -rf *.o $(TARGET)