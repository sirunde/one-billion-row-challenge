#include <iostream> 
// #include <unordered_map>
#include <vector>
#include <thread> //threading
#include <algorithm> //min max
#include <string>
// mmap
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "parallel_hashmap/phmap.h"

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

int conversion(char*& input){ // -0.1 -.1 -1.0 11.0
    int mod =1;
    if(*input == '-'){
        mod = -1;
        input++;
    }
    if(*input == '.'){
        input+=2;
        return (input[-1]-48)*mod;
    }
    if(input[1] == '.'){
        input+=4;
        return mod*((input[-2]-48)+((input[-4]-48)*10));
    }
    input+=5;
    return ((input[-2]-48)+((input[-4])-48)*10+(input[-5]-48)*100)* mod;
}

// it seperate files into 8 chunks, because it has 8 CPUs
chunk* seperate_chunk(int& fd, off_t& fsize,const int& cpus){

    // seperate chunks by 8, since thread is 8
    int cpu = cpus;
    off_t  perChunk = fsize/cpu; // perChunk = 40, fsize = 121, cpu = 3
    chunk *chunks = new chunk[cpu];
    
    int next = 0;
    off_t starting = 0;

    char* buffer = (char*)(mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0)); // assigned values in chunks
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
chunk* open_file(const char*& fileName,const int& cpu){
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
void ReadFile(chunk* chunks, const int& cpu, phmap::parallel_flat_hash_map<std::string, result>*& temp){
    phmap::parallel_flat_hash_map<std::string, result>* OneB = new phmap::parallel_flat_hash_map<std::string, result>;
    char* starting = &chunks[cpu].data[chunks[cpu].start];
    char* naming = starting;
    char* end = &chunks[cpu].data[chunks[cpu].end];
    int length;
    std::string name;
    while(starting < end){
        naming = starting;
        while(*starting != ';'){
            starting++;
        }
        length = starting-naming;
        ++starting;
        std::string name(naming,length);
        int t = conversion(starting);
        auto z = OneB->find(name);
        if(z != OneB->end()){
            z->second.min = std::min(z->second.min, t);
            z->second.total += t;
            z->second.max = std::max(z->second.max, t);
            z->second.num++;
        }
        else{
            OneB->emplace(name,result{t,t,t,1});
            // (*OneB)[name].max = t;
            // (*OneB)[name].min = t;
            // (*OneB)[name].total = t;
            // (*OneB)[name].num = 1;
        }
    }

    temp = OneB;
}

void threading(chunk* chunk, const int& cpu){
    std::vector<phmap::parallel_flat_hash_map<std::string, result>*> temp(cpu);
    phmap::parallel_flat_hash_map<std::string, result> OneB;
    std::thread myThreads[cpu];
    for (int i=0; i<cpu; i++){
        myThreads[i] = std::thread(ReadFile,chunk, i,std::ref(temp[i]));
        // ReadFile(chunk,i,std::ref(temp[i]),cpu);
    }
    for (int i=0; i<cpu; i++){
        myThreads[i].join();
    }
    OneB = *temp.at(0);
    int idx = 0;
    for(auto& i:temp){
        if(idx == 0){
            idx++;
            continue;
        }
        for(auto& j:*i){
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
    // for(auto x = ordered.begin(); x != ordered.end(); ++x)
    //     printf("%s=%.1f/%.1lf/%.1f",x->first.c_str(), x->second.min/10.,x->second.total/(10.*x->second.num), x->second.max/10.);

    std::vector<phmap::parallel_flat_hash_map<std::string, result>::iterator> output;
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
        if (x== output.back()){
            printf("%s=%.1f/%.1lf/%.1f",x->first.c_str(), x->second.min/10.,x->second.total/(10.*x->second.num), x->second.max/10.);

        }
    }
    printf("}\n");
    delete temp[0];
}

int main(int argc, char* argv[]){
    std::ios::sync_with_stdio(false);
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
    delete[] chunk;
}
