## One Billon Row Challenge
The text file contains temperature values for a range of weather stations. Each row is one measurement in the format <string: station name>;<double: measurement>

mmain.cpp used mmap
pmain.cpp used pread.

g++ -lpthread -g -Wall -Wextra mmain.cpp -o main -O3 used to create executable file.

default is 4 threads, but can be changed, ./main "file name" "number of threads"

![image](https://github.com/bu-cs447-2024-1s/one-billion-row-challenge-sirunde/assets/68354747/448bc580-becf-4dbf-900a-bd76ce34dc9a)
best 1B took 20.21s.
