CC=g++
TARGET=main
all: $(TARGET)

run:
	./main input.txt output.txt
	
$(TARGET): main.cpp
	$(CC) -g -Wall -Wextra main.cpp -o $@

clean:
	rm -rf *.o $(TARGET)