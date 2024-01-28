#include <iostream> 
#include <map>
#include <thread>

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
    double mean;
    double max;
};

struct chunk{
    size_t start=0;
    size_t end=0;
    char* data;
};

chunk* seperate_chunk(int fd, off_t fsize){
    // seperate chunks by 8, since thread is 8
    off_t  perChunk = fsize/8;
    chunk *chunks = new chunk[8];

    // to check next start point and end point
    int next = 0;
    int total = fsize/8;
    /* ex) 999, 999//8 = 124
        each chunk has 124
        (0-123), (124-247),(248-371), (372-495),(496-619), (620-743), (744-867), (868-992(999))

    */
    for(int i = 0;i<8;i++){
        // size of chunks
        chunks[i].end = perChunk-1+next;
        // if i == 0, start of chunks
        if(i==0){
            chunks[i].start = 0;
            chunks[i].data = new char[perChunk+next];
        }
        else if(i==7){
            // 
            chunks[i].start = perChunk*i-next;
            chunks[i].end = fsize - chunks[i].start-1;
            chunks[i].data = new char[perChunk+next];
        }
        else{
            chunks[i].start = perChunk*i-next;
            chunks[i].data = new char[perChunk+next];
        }
        size_t num = pread(fd,chunks[i].data, chunks[i].end, chunks[i].start);

        if(chunks[i].data[chunks[i].end]!=('\n' || EOF)){
            for(size_t j= 0; j<chunks[i].end;j++){
                if(chunks[i].data[chunks[i].end-j] == '\n'){
                    next = j;
                    chunks[i].start = 0;
                    chunks[i].end -= next;
                    break;
                }
            }
        }
        else{
            next = 0;
        }
        // std::cout << i << " " << chunks[i].data[chunks[i].end] << std::endl;
        
    }
    return chunks;
}

void open_file(const char* fileName){
    // was trying to use unordered map, but it needs to be sorted.
    std::map<std::string, result>* OneB = new std::map<std::string, result>;
    int fd = open(fileName,O_RDONLY);
    off_t fsize;
    fsize = lseek(fd, 0, SEEK_END);
    if(fd == -1){
        std::cout << "failed to read file\n";
        exit(0);
    }
    chunk *chunks = seperate_chunk(fd, fsize);


    close(fd);
}

void ReadFile(){

}

int main(int argc, char* argv[]){
    const char *file_name = argv[1];
    open_file(file_name);
    
}