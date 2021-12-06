#include <sys/mman.h>
#include <unistd.h>
#include<fcntl.h>
int main(int argc, char * argv[]){
    int fd = open("./hashtable.c",O_RDONLY);
    void * addr = mmap(NULL,1048576,PROT_READ,MAP_SHARED,fd,0);
    pause();
}