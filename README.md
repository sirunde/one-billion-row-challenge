## One Billon Row Challenge
The text file contains temperature values for a range of weather stations. Each row is one measurement in the format <string: station name>;<double: measurement>

mmain.cpp used mmap
pmain.cpp used pread.

g++ -pthread -g -Wall -Wextra mmain.cpp -o main -O3 used to create an executable file.

default is 4 threads, but can be changed, ./main "file name" "number of threads"

![image](https://github.com/bu-cs447-2024-1s/one-billion-row-challenge-sirunde/assets/68354747/448bc580-becf-4dbf-900a-bd76ce34dc9a)
best case of 1B took 20.21s using 8threads.
best case of 100M took 1.5s using 8threads.


First tried to use pread, because heard pread has better performance for sequential memory accessing and loading large files. However, it turns out there is no difference, or sometimes worse. Therefore, Used mmap.

Tried direct modification on mmap. It was fine for the small size of the file(100m), but when it was 1B, I could not load the file using mmap.

Tried to create a custom hash, but it turns out there is no difference on runtime, or even worse. Therefore, I revoked my work and stayed on the unordered map.
stob takes a lot of time so changed it to custom conversion. first, save it as int and then divide by 10. to give double.
found out unordered map is slow, so I changed the unordered map to a custom map, called parallel_flat_hash_map. It increased speed.
Tried to learn about SIMD, but did not have enough time to learn.
