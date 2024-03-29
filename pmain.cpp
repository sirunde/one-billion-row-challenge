#include <iostream> 
#include <unordered_map>
#include <chrono> //time
#include <vector>
#include <bits/stdc++.h>
#include <thread> //threading
#include <algorithm> //min max
#include <string>
// mmap
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "robin_hood.h"
// store results of each station
struct result{
    int min;
    long total;
    int max;
    long num;
};

struct chunk{
    char* data;
    size_t end = 0;
    size_t start = 0;
};

int conversion(char*& input){ // -0.1 -.1 -1.0 1.0
    int mod =0;
    if(*input == '-'){
        mod = -1;
        input++;
    }
    if(*input == '.'){
        input++;
        return (input[0]-48);
    }
    else{
        while(*input !='.'){
            mod*=10;
            mod+=(input[0]-48);
            input++;
        }
        input+=3;
        return mod*10+(input[-2]-48);
    }
}

// it seperate files into 8 chunks, because it has 8 CPUs
chunk* seperate_chunk(int& fd, off_t& fsize,const int& cpus){
    off_t  perChunk = fsize/cpus; // perChunk = 40, fsize = 121, cpu = 3
    chunk *chunks = new chunk[cpus];
    char* buffer = new char[fsize];
    int next = 0;
    int starting = 0;

    size_t num = pread(fd, buffer, fsize, starting); // assigned values in chunks
    for(int i = 0;i<cpus;i++){
        chunks[i].start = starting;
        chunks[i].end = perChunk*(i+1)+next;
        chunks[i].data = buffer;
        if(i == (cpus-1)){ // 1st check [39], and if it is \n
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
inline chunk* open_file(const char*& fileName,const int& cpu){
    // was trying to use unordered map, but it needs to be sorted.
    off_t fsize;
    int fd = open(fileName,O_RDONLY);
    fsize = lseek(fd, 0, SEEK_END);
    if(fd == -1){
        printf("failed to read file\n");
        exit(0);
    }
    chunk *chunks = seperate_chunk(fd, fsize,cpu);
    close(fd);
    return chunks;
}


// need thread running
void ReadFile(chunk* chunks, const int& cpu, robin_hood::unordered_map<std::string, result>*& temp){
    robin_hood::unordered_map<std::string, result>* OneB = new robin_hood::unordered_map<std::string, result>;
    char* starting = &chunks[cpu].data[chunks[cpu].start];
    char* naming = starting;
    std::string name = starting;
    char* end = &chunks[cpu].data[chunks[cpu].end];
    while(starting < end){
        naming = starting;
        while(*starting != ';'){
            starting++;
        }
        *starting = '\0';
        name = naming;
        ++starting;
        int t = conversion(starting);
        auto z = OneB->find(name);
        if(z != OneB->end()){
            z->second.min = std::min(z->second.min, t);
            z->second.total += t;
            z->second.max = std::max(z->second.max, t);
            z->second.num++;
        }
        else{
            (*OneB)[name] = result{t,t,t,1};
        }
    }
    
    // for(size_t j = chunks[cpu].start; j < chunks[cpu].end+1;j++){
    //     if((chunks[cpu].data[j] == 0) || chunks[cpu].data[j] == '\n'){
    //         if(line == ""){
    //             break;
    //         }
    //         num = line;
    //         line.clear();
    //         auto z = OneB->find(name);
    //         double t = conversion(num.c_str());
    //         if(z != OneB->end()){
    //             z->second.min = std::min(z->second.min, t);
    //             z->second.total += t;
    //             z->second.max = std::max(z->second.max, t);
    //             z->second.num++;
    //         }
    //         else{
    //             (*OneB)[name] = result{t,t,t,1.};
    //         }
    //     }
    //     else if(chunks[cpu].data[j] ==';'){
    //             name = line;
    //             line.clear();
    //     }
    //     else{
    //         line+=chunks[cpu].data[j];
    //     }
    // }
    temp = OneB;
}

void threading(chunk* chunk, const int& cpu){
    std::vector<robin_hood::unordered_map<std::string, result>*> temp(cpu);
    robin_hood::unordered_map<std::string, result> OneB;
    std::thread myThreads[cpu];
    for (int i=0; i<cpu; i++){
        myThreads[i] = std::thread(ReadFile,chunk, i,std::ref(temp[i]));
    }
    for (int i=0; i<cpu; i++){
        myThreads[i].join();
    }
    for(auto i:temp){
        for(auto j:*i){
            auto z = OneB.find(j.first);
            if(z != OneB.end()){
                z->second.min = std::min(z->second.min, j.second.min);
                z->second.total += j.second.total;
                z->second.max = std::max(z->second.max, j.second.max);
                z->second.num += j.second.num;
            }
            
            else{
                (OneB)[j.first] = j.second;
            }
        }
        
        delete i;
    }
    // std::map<std::string, result> ordered(OneB.begin(), OneB.end());
    // for(auto x = ordered.begin(); x != ordered.end(); ++X_OK)
    //     printf("%s %f %f %f",x->first.c_str(), x->second.min,x->second.total/x->second.num, x->second.max);
    std::vector<decltype(OneB)::iterator> output;
    output.reserve(OneB.size());
    for(auto it = OneB.begin(); it != OneB.end(); ++it)
        output.push_back(it);

    std::sort(output.begin(), output.end(),
              [](auto& lhs, auto&rhs) {
                  return lhs->first < rhs->first;
              });
    printf("{");
    for(auto const& x:output){
        printf("%s=%.1f/%.1lf/%.1f, ",x->first.c_str(), x->second.min/10.,x->second.total/(10.*x->second.num), x->second.max/10.);
    }
    printf("}\n");
}

int main(int argc, char* argv[]){
    std::ios::sync_with_stdio(false);
    auto start = std::chrono::high_resolution_clock::now();
    int cpu = 4;
    const char *file_name;
    if (argc > 1){
        file_name = argv[1];
        if (argc > 2)
            cpu = std::atoi(argv[2]);
    }
    else{
        printf("Usage: ./main file nthread");
        exit(0);
    }
    chunk* chunk = open_file(file_name,cpu);
    threading(chunk,cpu);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(stop - start);
    printf("Time taken: %f \n", duration.count());
    delete[] chunk;
}