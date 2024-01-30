#include <iostream> 
#include <map>
#include <unordered_map>
#include <thread> //threading
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
    size_t start=0;
    size_t end=0;
    char* data;
};

// it seperate files into 8 chunks, because it has 8 CPUs
chunk* seperate_chunk(int fd, off_t fsize,int cpu){
    // seperate chunks by 8, since thread is 8
    off_t  perChunk = fsize/cpu; // perChunk = 40, fsize = 121, cpu = 3
    chunk *chunks = new chunk[cpu];
    int next = 0;
    // to check next start point and end point
    /* ex) 999, 999//8 = 124
        each chunk has 124
        (0-123), (124-247),(248-371), (372-495),(496-619), (620-743), (744-867), (868-992(999))
    */
    for(int i = 0;i<cpu;i++){
        // size of chunks
        chunks[i].end = perChunk+next; // 1st. end = 40+0. 2nd, next was 4, so 40+4, total 44 size.
        // if i == 0, start of chunks
        if(i==0){
            chunks[i].start = 0; 
            chunks[i].data = new char[chunks[i].end+1]; // data size = 41

        } // size = 41, bytes 0-39, 40 is going to be \000
        else if(i==(cpu-1)){
            // 
            chunks[i].start = chunks[i-1].end+1+chunks[i-1].start ;
            chunks[i].end = fsize - chunks[i].start+1; // fsize = 10, start = 0, end = 10
            chunks[i].data = new char[chunks[i].end +1];
        }
        else{
            chunks[i].start = chunks[i-1].end+1+chunks[i-1].start; // start from 40-4+1 = 37 which is after \n 37+45-9 = 
            chunks[i].data = new char[chunks[i].end +1]; //40+4+1 = 45, is fine
        }
        size_t num = pread(fd,chunks[i].data, chunks[i].end, chunks[i].start); // assigned values in chunks
        if(i == (cpu-1) || chunks[i].data[chunks[i].end-1] == '\n'){ // 1st check [39], and if it is \n
            next = -1; // if so, set next = 0
            chunks[i].end--;
        }
        else{
            for(size_t j= 0; j<chunks[i].end;j++){
                if(chunks[i].data[chunks[i].end-j] == '\n'){ //1st check, 36 was \n, so next is 4.
                    next = j; // next = 4
                    chunks[i].end -= next; // descrese end to 36, so we hae \n at the end of line
                    break;
                }
            }
        }
        // std::cout << i << " " << chunks[i].data[chunks[i].end] << std::endl;
        
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
void ReadFile(chunk* chunks, int cpu){
    std::unordered_map<std::string, result> OneB;
    std::string line ="";
    std::string name = "";
    std::string num = "";
    for(auto i=0;i<cpu;i++){
        for(size_t j = 0; j < chunks[i].end+1;j++){
            if(chunks[i].data[j] ==';'){
                name = line;
                line.clear();
            }
            else if(chunks[i].data[j] == '\n'){
                num = line;
                line.clear();
                auto z = OneB.find(name);
                if(z != OneB.end()){
                    z->second.min = std::min(z->second.min, std::stod(num));
                    z->second.total += std::stod(num);
                    z->second.max = std::max(z->second.max, std::stod(num));
                    z->second.num++;
                }
                else{
                    OneB[name] = result{std::stod(num),std::stod(num),std::stod(num),1.};
                }
            }
            else if(chunks[i].data[j] == 0){
                                num = line;
                line.clear();
                auto z = OneB.find(name);
                if(z != OneB.end()){
                    z->second.min = std::min(z->second.min, std::stod(num));
                    z->second.total += std::stod(num);
                    z->second.max = std::max(z->second.max, std::stod(num));
                    z->second.num++;
                }
                else{
                    OneB[name] = result{std::stod(num),std::stod(num),std::stod(num),1.};
                }
                break;
            }
            else{
                line+=chunks[i].data[j];
            }
        }
    }

    for(auto const& x:OneB){
        std::cout << x.first << " " << x.second.min << " " << x.second.total/x.second.num << " " << x.second.max << " " << x.second.num << " ";
    }
}

int main(int argc, char* argv[]){
    const char *file_name = argv[1];
    int cpu = std::atoi(argv[2]);
    chunk* chunk = open_file(file_name,cpu);
    ReadFile(chunk, cpu);
}