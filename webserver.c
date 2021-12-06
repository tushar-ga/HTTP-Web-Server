#include "webserver.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include<sys/shm.h>
pthread_mutex_t msgqLock = PTHREAD_MUTEX_INITIALIZER;

#define respMsgHeaderTemp "HTTP/1.1 %d %s\r\nDate: %s\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n"

void *processClients( void *input){
    char cgifile[100];
    char cgiargs[200];
    int msqid = *(int *)input;
    msgqbuf msg;

    while(true){
        
        Pthread_mutex_lock(&msgqLock);
            Msgrcv(msqid, &msg, sizeof(int), /* msgtyp = */ 0, /* msgflags = */ 0);
        Pthread_mutex_unlock(&msgqLock);
        
        clientConnData *clientPtr = searchTable(clientConnDataTable, msg.sockfd);

        if(clientPtr == NULL){
            continue;
        }

        state clientState = clientPtr->currState;
        printf("Received Original message for file %s at state: %d\n", clientPtr->path, clientState);
        if(msg.fromChild == 1|| clientState==FILE_MAPPING_IN_PROGRESS){
            msg.fromChild = 0;
            clientState = READ_FILE;
        }
        printf("Received message for file %s at state: %d\n", clientPtr->path, clientState);
        switch(clientState){
            
            case REQ_READ:
            {
                printf("reading request\n");
                int numRead = Read(msg.sockfd, clientPtr->friptr, &(clientPtr->fr[BUFFERSIZE]) - clientPtr->friptr);
                printf("req is : %s\n", clientPtr->fr);
                if(numRead == 0){
                    Close(msg.sockfd);
                    continue;
                }
                else if(numRead < 0){
                    if(errno != ECONNRESET && errno != EBADF)
                        {printf("Sending the message for file %s at state:%d\n",clientPtr->path,clientPtr->currState);
                        Msgsnd(msqid, &msg, sizeof(int), /* msgflags = */ 0);}
                    continue;
                }
                else{
                    clientPtr->friptr += numRead;
                    char *endPtr = clientPtr->friptr - 1;

                    *(clientPtr->friptr) = '\0';

                    if( (clientPtr->friptr - clientPtr->fr >= 4) && (*endPtr == '\n') && (*(endPtr - 1) == '\r') && (*(endPtr - 2) == '\n') && (*(endPtr - 3) == '\r') ){                
                        clientPtr->currState = GET_FILE_NAME;       
                    }                    
                }
            }
            break;

            case GET_FILE_NAME:
            {
                char filepath[FILENAME_LEN];
                char firstLine[LINE_SIZE];
                printf("Getting File name\n");
                sscanf(clientPtr->fr, "%[^\n]", firstLine);
                filepath[0] = '.';  // relative path
                sscanf(firstLine + /* skipping 'GET '*/4, "%s", filepath + 1 /*1 to skip the .*/);
                
                strncpy(clientPtr->path, filepath, FILENAME_LEN);
                
                int cgi = is_cgi(clientPtr->path,cgifile,cgiargs);
                if(cgi){
                    clientPtr->currState = CGI_PROCESSING;
                }
                else clientPtr->currState = MAPPING_FILE;
            }
            break;
            case CGI_PROCESSING:
                {
                    int sock = Socket(AF_LOCAL,SOCK_STREAM,0);
                    struct sockaddr_un servaddr;
                    servaddr.sun_family = AF_LOCAL;
                    strcpy(servaddr.sun_path, "/tmp/fastcgiserverNP");
                    int connfd = connect(sock,(struct sockaddr*)&servaddr,sizeof(servaddr));
                    if(connfd==-1) {perror("connect");}
                    else {
                        FCGIreq req;
                        strcpy(req.filename,cgifile);
                        strcpy(req.args,cgiargs);
                        sock_fd_write(sock,&req,sizeof(req),msg.sockfd);
                        removeFromTable(clientConnDataTable, msg.sockfd);
                    }
                }
            break;
            case MAPPING_FILE:
            {
                
                printf("Mapping File\n");
                if(clientPtr->fd == -2)
                    clientPtr->fd = Open(clientPtr->path, O_RDONLY);

                if(clientPtr->fd < 0){
                    strcpy(clientPtr->path, "./404.html");
                    clientPtr->fileExists = false;
                    clientPtr->fd = Open(clientPtr->path, O_RDONLY);
                }
                else{                    
                    clientPtr->fileExists = true;                    
                }
                
               
                struct stat st;
                fstat(clientPtr->fd, &st);
                clientPtr->size = st.st_size;
                clientPtr->offset = 0;
                clientPtr->currState = FILE_MAPPING_IN_PROGRESS;
                int key = ftok(clientPtr->path,msg.sockfd);
                int shm = Shmget(key,20485760, IPC_CREAT|0666);
                 printf("Attaching Memory, shm:%d\n",shm);
                clientPtr->memAddr = Shmat(shm,0,0);
                clientPtr->shm = shm;
                //helper process to map file
                if(fork()==0){
                    printf("Getting shm\n");
                    int key = ftok(clientPtr->path,msg.sockfd);
                    int shm = Shmget(key,20485760, 0666);               
                    printf("Attaching shm\n");
                    char * addr = Shmat(shm,0,0);
                    printf("Mmap at shm\n");
                    char * faddr = mmap(addr,20485760,PROT_READ,MAP_SHARED,clientPtr->fd,0);
                    for(int i=0;i<clientPtr->size;i++){
                        addr[i] = faddr[i];
                    }
                    // printf("%s\n",(char *)addr);
                    // if((void *)faddr!=MAP_FAILED){
                    //     clientPtr->currState = READ_FILE;
                    // }
                    msg.fromChild =1;
                     printf("Sending the message for file %s at state:%d from Child\n",clientPtr->path,clientPtr->currState);
                    Msgsnd(msqid, &msg, sizeof(int), /* msgflags = */ 0);
                    exit(0);
                }
                
                // printf("Reading diskfile %s\n", clientPtr->path);

                // int numRead = Read(clientPtr->fd, clientPtr->to, BUFFERSIZE);

                // clientPtr->numRead = numRead;
                
                // clientPtr->toiptr += numRead;

                // if(clientPtr->writtenHeader == false){
                //     int currOffset = Lseek(clientPtr->fd, 0, SEEK_CUR);
                    
                //     clientPtr->payloadSize = Lseek(clientPtr->fd, 0, SEEK_END);
                //     Lseek(clientPtr->fd, currOffset, SEEK_SET);

                //     if(currOffset == clientPtr->payloadSize){
                //         clientPtr->readCompletely = true;                        
                //     }

                //     clientPtr->currState = WRITING_HEADER;
                // }
                // else{
                //     int currOffset = Lseek(clientPtr->fd, 0, SEEK_CUR);

                //     if(currOffset == clientPtr->payloadSize){
                //         clientPtr->readCompletely = true;
                //     }

                //     clientPtr->currState = WRITING_BODY;
                // }

                // if(clientPtr->readCompletely)
                //     Close(clientPtr->fd);
            }
            break;

            case READ_FILE:
                {
                int leftToread = clientPtr->size - clientPtr->offset;
                int sz = (leftToread) < BUFFERSIZE ? leftToread : BUFFERSIZE;
                printf("Copying from shm\n");
                int PAGE_SIZE = sysconf(_SC_PAGESIZE);
                unsigned char vec[(20485760+PAGE_SIZE-1)/PAGE_SIZE];
                mincore(clientPtr->memAddr+clientPtr->offset,20485760,&vec);
                if((vec[0]&1)==1) printf("Resident in memory\n");
                strncpy(clientPtr->to,(clientPtr->memAddr+clientPtr->offset),sz);
                printf("Copied data\n%d\n%s\n",clientPtr->size,clientPtr->path);
                printf("Copied from shm\n");
                clientPtr->offset +=sz;
                clientPtr->toiptr += sz;
                    if(clientPtr->offset == clientPtr->size){
                            clientPtr->readCompletely = true;
                        }

                    if(clientPtr->writtenHeader == false){                        
                        clientPtr->currState = WRITING_HEADER;
                        }
                    else{
                       clientPtr->currState = WRITING_BODY;
                    }
            }

            break;
            case WRITING_HEADER:
            {
                printf("Writing header\n");

                if(clientPtr->fileExists)
                    snprintf(clientPtr->hdriptr, BUFFERSIZE, respMsgHeaderTemp, 200, "OK", getTimestamp(),
                                getMimeType(clientPtr->path + 2 /*2 is to skip ./ */),clientPtr->size);
                else
                    snprintf(clientPtr->hdriptr, BUFFERSIZE, respMsgHeaderTemp, 404, "Not Found", getTimestamp(),
                                getMimeType(clientPtr->path + 2 /*2 is to skip ./ */),clientPtr->size);

                clientPtr->hdriptr += strlen(clientPtr->hdriptr);

                int numWritten = Write(msg.sockfd, clientPtr->hdroptr, clientPtr->hdriptr - clientPtr->hdroptr);

                if(numWritten < 0) {
                    if(errno != ECONNRESET && errno != EBADF)
                        {printf("Sending the message for file %s at state:%d\n",clientPtr->path,clientPtr->currState);
                        Msgsnd(msqid, &msg, sizeof(int), /* msgflags = */ 0);}
                    continue;
                }
                else if(numWritten == 0) {
                    Close(msg.sockfd);
                    continue;
                }
                else{
                    clientPtr->hdroptr += numWritten;
                    if(clientPtr->hdroptr == clientPtr->hdriptr){
                        clientPtr->hdriptr = clientPtr->hdroptr = clientPtr->header;
                        clientPtr->currState = WRITING_BODY;
                        clientPtr->writtenHeader = true;
                    }
                }
            }
            break;

            case WRITING_BODY:
            {   
                printf("Writing body\n");
                
                int numWritten = Write(msg.sockfd, clientPtr->tooptr, clientPtr->toiptr - clientPtr->tooptr);
                
                if(numWritten < 0){
                    if(errno != ECONNRESET && errno != EBADF)
                         {printf("Sending the message for file %s at state:%d\n",clientPtr->path,clientPtr->currState);
                        Msgsnd(msqid, &msg, sizeof(int), /* msgflags = */ 0);}
                    continue;
                }
                else if(numWritten == 0) {
                    Close(msg.sockfd);
                    continue;
                }
                else{
                    clientPtr->tooptr += numWritten;
                    if(clientPtr->tooptr == clientPtr->toiptr){
                        clientPtr->toiptr = clientPtr->tooptr = clientPtr->to;
                        if(clientPtr->readCompletely)
                            clientPtr->currState = DONE;
                        else
                            clientPtr->currState = READ_FILE;
                    }
                }
            }
            break;

            case DONE:
            {
                strcpy(clientPtr->fr, "");
                strcpy(clientPtr->to, "");
                printf("in done state\n");
                Shmctl(clientPtr->shm,IPC_RMID,NULL);
                removeFromTable(clientConnDataTable, msg.sockfd);
                continue;
            }
            break;
            default:
            {
                printf("It should not have come here!! Something's wrong.., state:%d\n",clientPtr->currState);
                exit(EXIT_FAILURE);
            }
        }
        if((clientPtr->currState)!=4) {
            printf("Sending the message for file %s at state:%d\n",clientPtr->path,clientPtr->currState);
            Msgsnd(msqid, &msg, sizeof(int), /* msgflags = */ 0);
            }
    }

    return NULL;
}

int main(){
    if(fork()==0){ //starting fcgi process
        execl("./fcgi","./fcgi",(char*)NULL);
    }
    initTable(clientConnDataTable);
    pthread_t thread[NUMTHREADS];

    int msgqid = Msgget(IPC_PRIVATE, 0666);

    for(int i = 0; i < NUMTHREADS; i++){
        Pthread_create(&thread[i], NULL, processClients, (void *) &msgqid);
    }

    int epfd = Epoll_create1(0); 

    int listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverAddr, clientAddr;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);

    // for allowing binding again
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        errorExit("setsockopt(SO_REUSEADDR) failed");

    Bind(listenfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

     if(listen(listenfd, MAX_QUEUE_SIZE) < 0){
        errorExit("listen failed");
    }
    struct epoll_event ev;
    ev.data.fd = listenfd;
    ev.events = EPOLLIN | EPOLLET;

    Epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

    struct epoll_event readyList[NUMTHREADS];
    int numReady = 0;
    int connfd = -1;
    struct msgqbuf msg;
    socklen_t cliLen = sizeof(clientAddr);

    while(true){
        numReady = Epoll_wait(epfd, readyList,/* maxevents = */ 20,/* timeout = */ -1);
        
        for(int i = 0; i < numReady; i++){

            if( (readyList[i].data.fd == listenfd) && (readyList[i].events & EPOLLIN) ){
                connfd = Accept(listenfd, (struct sockaddr *) &clientAddr, &cliLen);
                struct epoll_event event;
                event.data.fd = connfd;
                event.events = EPOLLIN | EPOLLET;

                int status = fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK);

                if (status == -1){
                    errorExit("fcntl failed");
                }
                Epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &event);
                
            }
            else if(readyList[i].events & EPOLLIN) {
                                
                clientConnData *clientPtr;
                clientPtr = (clientConnData *) malloc(sizeof(clientConnData));
                clientPtr->currState = REQ_READ;
                clientPtr->friptr = clientPtr->froptr = clientPtr->fr;
                clientPtr->toiptr = clientPtr->tooptr = clientPtr->to;
                clientPtr->hdriptr = clientPtr->hdroptr = clientPtr->header;
                clientPtr->fd = -2;
                clientPtr->writtenHeader = clientPtr->fileExists = clientPtr->readCompletely = false;
                clientPtr->size = 0;

                strcpy(clientPtr->path, "");
                strcpy(clientPtr->reqLastfourCharsStr, "");

                insertInTable(clientConnDataTable, readyList[i].data.fd, clientPtr);

                msg.mtype = 100;
                msg.sockfd = readyList[i].data.fd;
                msg.fromChild = 0;
                Msgsnd(msgqid, &msg, sizeof(int), /* flag = */ 0);
            }
        }
    }
}