#include "helper.h"

void errorExit(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

void errorExitPthread(char *msg, int errnum){
    printf("%s : %s\n",msg, strerror(errnum));
    exit(EXIT_FAILURE);
}

int Msgget(key_t key, int msgflg){
    int msqid = msgget(key, msgflg);
    if( msqid < 0){
        errorExit("msgget failed");
    }

    return msqid;
}

int Msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg){
    if(msgsnd(msqid, msgp, msgsz, msgflg) < 0){
        errorExit("msgsnd failed");
    }

    return SUCCESS;
}

ssize_t Msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg){
    ssize_t numrcvd = msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
    if( numrcvd < 0){
        errorExit("msgrcv failed");
    }

    return numrcvd;
}

int Msgctl(int msqid, int cmd, struct msqid_ds *buf){
    if(msgctl(msqid, cmd, buf) < 0){
        errorExit("msgcrl failed");
    }

    return SUCCESS;
}

int Socket(int domain, int type, int protocol){

    int sockfd = socket(domain, type, protocol);

    if( sockfd < 0){
        errorExit("socket failed");
    }

    return sockfd;
}


int Listen(int sockfd, int backlog){
    if(listen(sockfd, backlog) < 0){
        errorExit("listen failed");
    }

    return SUCCESS;
}

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    if(bind(sockfd, addr, addrlen) < 0){
        errorExit("bind failed");
    }

    return SUCCESS;
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    int connfd = accept(sockfd, addr, addrlen);
    if( connfd < 0){
        errorExit("accept failed");
    }

    return connfd;
}

int Open(const char *pathname, int flags){
    int fd = open(pathname, flags);

    // if(fd < 0){
    //     errorExit("open failed");
    // }

    return fd;
}

ssize_t Read(int fd, void *buf, size_t count){
    printf("reading count = %ld\n", count);
    int numRead = read(fd, buf, count);
    if( numRead < 0){
        if(errno != EWOULDBLOCK && errno != EAGAIN){
            if(errno != ECONNRESET && errno != EBADF)
                errorExit("read failed");
        }
    }

    return numRead;
}

ssize_t Write(int fd, const void *buf, size_t count){
    int numWritten = write(fd, buf, count);
    
    if(numWritten < 0){
        if(errno != EWOULDBLOCK && errno != EAGAIN){
            if(errno != ECONNRESET && errno != EBADF)
                errorExit("write failed");
        }
    }

    return numWritten;
}

int Close(int fd){

    printf("Closing %d\n", fd);

    if(close(fd) < 0 ){
        if(errno != EBADF)
            errorExit("close failed");
    }

    return SUCCESS;
}

off_t Lseek(int fd, off_t offset, int whence){
    off_t offsetVal = lseek(fd, offset, whence);

    if(offsetVal < 0){
        errorExit("lseek failed");
    }

    return offsetVal;
}

int Epoll_create1(int flags){
    int epfd = epoll_create1(flags);

    if(epfd < 0){
        errorExit("epoll_create failed");
    }

    return epfd;
}

int Epoll_ctl(int epfd, int op, int fd, struct epoll_event *ev){
    if(epoll_ctl(epfd, op, fd, ev) < 0){
        errorExit("epoll_ctl failed");
    }

    return SUCCESS;
}

int Epoll_wait(int epfd, struct epoll_event *evlist, int maxevents, int timeout){
    int num_epfds = epoll_wait(epfd, evlist, maxevents, timeout);

    if(num_epfds < 0){
        errorExit("epoll_wait failed");
    }

    return num_epfds;
}

int Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg){
    int status = pthread_create(thread, attr, start_routine, arg);
    
    if(status > 0){
        errorExitPthread("pthread_create failed", status);
    }

    return SUCCESS;
}

int Pthread_join(pthread_t thread, void **retval){
    int status = pthread_join(thread, retval);

    if(status > 0){
        errorExitPthread("pthread_create failed", status);
    }

    return SUCCESS;
}

int Pthread_detach(pthread_t thread){

    int status = pthread_detach(thread);

    if(status > 0){
        errorExitPthread("pthread_create failed", status);
    }

    return SUCCESS;
}

pthread_t Pthread_self(void){
    return pthread_self();
}

void Pthread_exit(void *retval){
    pthread_exit(retval);
}

int Pthread_mutex_lock(pthread_mutex_t *mutex){

    int status = pthread_mutex_lock(mutex);

    if(status > 0){
        errorExitPthread("pthread_create failed", status);
    }

    return SUCCESS;

}

int Pthread_mutex_unlock(pthread_mutex_t *mutex){

    int status = pthread_mutex_unlock(mutex);

    if(status > 0){
        errorExitPthread("pthread_create failed", status);
    }

    return SUCCESS;

}

char *tolowerStr(char *str){
    int len = strlen(str);

    for(int i = 0; i < len; i++){
        str[i] = tolower(str[i]);
    }

    return str;
}

char *getMimeType(char *filename){
    char *extension = strchr(filename, '.');

    if(extension == NULL){
        return DEFAULT_MIME_TYPE;
    }

    extension = extension + 1; /* skip . */
    extension = tolowerStr(extension);

    printf("extension is %s\n", extension);
    if(strcmp(extension,"cgi")==0){
        return "cgi";
    }
    
    if (strcmp(extension, "jpeg") == 0 || strcmp(extension, "jpg") == 0) { 
        return "image/jpg"; 
    }
    if (strcmp(extension, "gif") == 0) { 
        return "image/gif"; 
    }
    if (strcmp(extension, "json") == 0) { 
        return "application/json"; 
    }
    if (strcmp(extension, "png") == 0) { 
        return "image/png"; 
    }
    if (strcmp(extension, "mp3") == 0) {
        return "audio/mpeg";
    }
    if (strcmp(extension, "css") == 0) { 
        return "text/css"; 
    }
    if (strcmp(extension, "html") == 0 || strcmp(extension, "htm") == 0) { 
        return "text/html"; 
    }
    if (strcmp(extension, "js") == 0) { 
        return "application/javascript"; 
    }
    if (strcmp(extension, "txt") == 0) { 
        return "text/plain"; 
    }
    return DEFAULT_MIME_TYPE;
}

char *getTimestamp(){
    char *buf = (char*)malloc(sizeof(char) * 1000);
    time_t now = time(0);

    struct tm tm = *gmtime(&now);
    strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    
    return buf;
}

int Shmget(key_t __key, size_t __size, int __shmflg){
    int val = shmget(__key,__size,__shmflg);
    if(val==-1){
        perror("shmget");
    }
    return val;
}

void *Shmat(int __shmid, const void *__shmaddr, int __shmflg){
    void * addr = shmat( __shmid, __shmaddr,  __shmflg);
    if(addr==(int *) -1){
        perror("shmat");
    }
    return addr;
}
int Shmctl(int __shmid, int __cmd, struct shmid_ds *__buf){
    int status = shmctl( __shmid, __cmd,  __buf);
    if(status<0){
        perror("shmctl");
    }
    return status;
}

ssize_t
sock_fd_read(int sock, void *buf, ssize_t bufsize, int *fd)
{
    ssize_t     size;

    if (fd) {
        struct msghdr   msg;
        struct iovec    iov;
        union {
            struct cmsghdr  cmsghdr;
            char        control[CMSG_SPACE(sizeof (int))];
        } cmsgu;
        struct cmsghdr  *cmsg;

        iov.iov_base = buf;
        iov.iov_len = bufsize;

        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = cmsgu.control;
        msg.msg_controllen = sizeof(cmsgu.control);
        size = recvmsg (sock, &msg, 0);
        if (size < 0) {
            perror ("recvmsg");
            exit(1);
        }
        cmsg = CMSG_FIRSTHDR(&msg);
        if (cmsg && cmsg->cmsg_len == CMSG_LEN(sizeof(int))) {
            if (cmsg->cmsg_level != SOL_SOCKET) {
                fprintf (stderr, "invalid cmsg_level %d\n",
                     cmsg->cmsg_level);
                exit(1);
            }
            if (cmsg->cmsg_type != SCM_RIGHTS) {
                fprintf (stderr, "invalid cmsg_type %d\n",
                     cmsg->cmsg_type);
                exit(1);
            }

            *fd = *((int *) CMSG_DATA(cmsg));
            printf ("received fd %d\n", *fd);
        } else
            *fd = -1;
    } else {
        size = read (sock, buf, bufsize);
        if (size < 0) {
            perror("read");
            exit(1);
        }
    }
    return size;
}

ssize_t
sock_fd_write(int sock, void *buf, ssize_t buflen, int fd)
{
    ssize_t     size;
    struct msghdr   msg;
    struct iovec    iov;
    union {
        struct cmsghdr  cmsghdr;
        char        control[CMSG_SPACE(sizeof (int))];
    } cmsgu;
    struct cmsghdr  *cmsg;

    iov.iov_base = buf;
    iov.iov_len = buflen;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if (fd != -1) {
        msg.msg_control = cmsgu.control;
        msg.msg_controllen = sizeof(cmsgu.control);

        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_len = CMSG_LEN(sizeof (int));
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;

        printf ("passing fd %d\n", fd);
        *((int *) CMSG_DATA(cmsg)) = fd;
    } else {
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
        printf ("not passing fd\n");
    }

    size = sendmsg(sock, &msg, 0);

    if (size < 0)
        perror ("sendmsg");
    return size;
}
int is_cgi(char *uri, char *filename, char *cgiargs)
{
 char *ptr;

    if (strstr(uri, "cgi-bin")) { /* Dynamic content */   
        ptr = index(uri, '?');
            if (ptr) {
                strcpy(cgiargs, ptr+1);
                *ptr = '\0';
                }
            else strcpy(cgiargs, "");
        strcpy(filename, uri);
        return 1;
    }
    return 0;
 }