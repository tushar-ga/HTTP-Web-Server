#include "stdio.h"
#include "helper.h"
extern char **environ;
int main(int argc, char * argv[]){
    char * emptylist[]= {NULL};
    int socket = Socket(AF_LOCAL,SOCK_STREAM,0);
    struct sockaddr_un servaddr,cliaddr;
    socklen_t clientsize = sizeof(cliaddr);
    servaddr.sun_family = AF_LOCAL;
    unlink("/tmp/fastcgiserverNP");
    strcpy(servaddr.sun_path, "/tmp/fastcgiserverNP");
    if (Bind(socket,(struct sockaddr *)&servaddr,sizeof(servaddr))!=1) perror("bind");
    Listen(socket,10);
    for(;;){
        int connfd = Accept(socket,(struct sockaddr *)&cliaddr,&clientsize);
        if(connfd<0){
            perror("accept");
            continue;
        }
        if(fork()==0){
            Close(socket);
            FCGIreq req;
            int fd;
            sock_fd_read(connfd,&req,sizeof(req),&fd);
            Close(1);
            dup(fd);
            setenv("QUERY_STRING",req.args,1);
            printf("Running file%s",req.filename);
            execve(req.filename,emptylist,environ);
        }
        close(connfd);
    }
}