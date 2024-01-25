#include <iostream> 
#include <map>
#include <thread>
#include <fstream>

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

void open_file(const char* fileName){
    // was trying to use unordered map, but it needs to be sorted.
    std::map<std::string, result>* OneB = new std::map<std::string, result>;
    // int num = 0;
    // int f = open(fileName, O_RDONLY);
    // off_t size = lseek(f, 0, SEEK_END);
    // char * buffer = (char *)mmap(NULL,size, PROT_READ, MAP_PRIVATE, f, 0);

}

int main(){
    std::ofstream myfile;
    const char *file_name = "test.txt";
    open_file(file_name);
}