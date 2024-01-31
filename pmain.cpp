#include <iostream> 
#include <map>
#include <unordered_map>
#include <chrono> //time
#include <vector>
#include <bits/stdc++.h>
#include <iterator>
#include <thread> //threading
#include <mutex> 

#include <algorithm> //min max
//ifstream
#include <fstream>
#include <sstream>
#include <string>
// mmap
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// store results of each station
struct result{
    double min;
    double total;
    double max;
    double num;
};

struct chunk{
    char* data;
    size_t end = 0;
    size_t start = 0;
};
// 0 123, 123 == \n, start from 124
// it seperate files into 8 chunks, because it has 8 CPUs
chunk* seperate_chunk(int fd, off_t fsize,int cpu){
    // seperate chunks by 8, since thread is 8
    off_t  perChunk = fsize/cpu; // perChunk = 40, fsize = 121, cpu = 3
    chunk *chunks = new chunk[cpu];
    char* buffer = new char[fsize];
    int next = 0;
    int starting = 0;

    size_t num = pread(fd, buffer, fsize, starting); // assigned values in chunks

    for(int i = 0;i<cpu;i++){
        chunks[i].start = starting;
        chunks[i].end = perChunk*(i+1)+next;
        chunks[i].data = buffer;
        if(i == (cpu-1)){ // 1st check [39], and if it is \n
            chunks[i].end = fsize;
            break;
        }
        else{
            for(size_t j= 0; j<chunks[i].end;j++){
                if(chunks[i].data[chunks[i].end-j] == '\n'){ //1st check, 36 was \n, so next is 4.
                    next = j; // next = 4
                    chunks[i].end -= next; // descrese chunks[i].end to 36, so we hae \n at the chunks[i].end of line
                    break;
                }
            }
        }
        starting = chunks[i].end+1;
        // std::cout << i << " " << chunks[i].data[chunks[i].chunks[i].end] << std::chunks[i].endl;
    }
    return chunks;
}

// open file, and run seperate chunk
chunk* open_file(const char* fileName,int cpu){
    // was trying to use unordered map, but it needs to be sorted.
    int fd = open(fileName,O_RDONLY);
    off_t fsize;
    fsize = lseek(fd, 0, SEEK_END);
    if(fd == -1){
        std::cout << "failed to read file\n";
        exit(0);
    }
    chunk *chunks = seperate_chunk(fd, fsize,cpu);
    close(fd);
    return chunks;
}


// need thread running
void ReadFile(chunk* chunks, int cpu, std::unordered_map<std::string, result>*& temp){
    std::unordered_map<std::string, result>* OneB = new std::unordered_map<std::string, result>;
    std::string line ="";
    std::string name = "";
    std::string num = "";
    for(size_t j = chunks[cpu].start; j < chunks[cpu].end+1;j++){
        if((chunks[cpu].data[j] == 0) || chunks[cpu].data[j] == '\n'){
            if(line == ""){
                break;
            }
            num = line;
            line.clear();
            auto z = OneB->find(name);
            if(z != OneB->end()){
                z->second.min = std::min(z->second.min, std::stod(num));
                z->second.total += std::stod(num);
                z->second.max = std::max(z->second.max, std::stod(num));
                z->second.num++;
            }
            
            else{
                (*OneB)[name] = result{std::stod(num),std::stod(num),std::stod(num),1.};
            }
        }
        else if(chunks[cpu].data[j] ==';'){
                name = line;
                line.clear();
        }
        else{
            line+=chunks[cpu].data[j];
        }
    }
    temp = OneB;
}

void threading(chunk* chunk, int cpu){
    std::vector<std::unordered_map<std::string, result>*> temp(cpu);
    std::unordered_map<std::string, result> OneB;
    std::thread myThreads[cpu];
    for (int i=0; i<cpu; i++){
        myThreads[i] = std::thread(ReadFile,chunk, i,std::ref(temp[i]));
    }
    for (int i=0; i<cpu; i++){
        myThreads[i].join();
    }
    for(auto i:temp){
        for(auto j:*i){
            OneB[j.first] = j.second;
        }
    }
    std::vector<decltype(OneB)::iterator> output;
    output.reserve(OneB.size());
    for(auto it = OneB.begin(); it != OneB.end(); ++it)
        output.push_back(it);

    std::sort(output.begin(), output.end(),
              [](auto& lhs, auto&rhs) {
                  return lhs->first < rhs->first;
              });
    

    for(auto const& x:output){
        std::cout << x->first << " " << x->second.min << " " << x->second.total/x->second.num << " " << x->second.max << " " << x->second.num << " ";
    }
}

int main(int argc, char* argv[]){
    auto start = std::chrono::high_resolution_clock::now();
    int cpu = 4;
    const char *file_name;
    if (argc > 1){
        file_name = argv[1];
        if (argc > 2)
            cpu = std::atoi(argv[2]);
    }
    else{
        std::cout << "Usage: ./main file nthread";
        exit(0);
    }
    chunk* chunk = open_file(file_name,cpu);
    threading(chunk,cpu);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(stop - start);
    std::cout << "Time taken: " << duration.count() << std::endl;
    delete[] chunk[0].data;
    delete[] chunk;
}