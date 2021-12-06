#ifndef HELPER_H
#define HELPER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for read() / write()
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <sys/shm.h>
#include"sys/un.h"
#define BUFFERSIZE 30000
#define LINE_SIZE 100
#define FILENAME_LEN 100
#define FCGI_ARGS_LEN 200
#define SUCCESS 1
#define DEFAULT_MIME_TYPE "application/octet-stream"

/* Client info */
typedef enum state {REQ_READ, GET_FILE_NAME, CGI_PROCESSING, MAPPING_FILE, FILE_MAPPING_IN_PROGRESS, READ_FILE, WRITING_HEADER, WRITING_BODY, DONE} state;
typedef enum {false, true} bool;


typedef struct clientConnData{
    state currState;
    char fr[BUFFERSIZE];
    char to[BUFFERSIZE];
    char path[FILENAME_LEN];
    char header[BUFFERSIZE];
    bool fileExists;
    int fd;
    int offset;
    int size;
    int shm;
    bool cgi;
    char * memAddr;
    char *friptr;
    char *froptr;
    char *toiptr;
    char *tooptr;
    char *hdriptr;
    char *hdroptr;
    int numRead;
    bool writtenHeader;
    bool readCompletely;
    char reqLastfourCharsStr[5];
} clientConnData;

/* Message queue functions */

typedef struct msgqbuf{
    long mtype;
    int sockfd;
    int fromChild;
} msgqbuf;

typedef struct FCGIreq{
    char filename[FILENAME_LEN];
    char args[FCGI_ARGS_LEN];
} FCGIreq; 
int Msgget(key_t key, int msgflg);

int Msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);

ssize_t Msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);

int Msgctl(int msqid, int cmd, struct msqid_ds *buf);


/* Exit on system call error function*/
void errorExit(char *msg);

void errorExitPthread(char *msg, int errnum);

/* Socket functions */

int Socket(int domain, int type, int protocol);

int Listen(int sockfd, int backlog);

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

int Open(const char *pathname, int flags);

ssize_t Read(int fd, void *buf, size_t count);

ssize_t Write(int fd, const void *buf, size_t count);

off_t Lseek(int fd, off_t offset, int whence);

int Close(int fd);

/* epoll functions */

int Epoll_create1(int flags);

int Epoll_ctl(int epfd, int op, int fd, struct epoll_event *ev);

int Epoll_wait(int epfd, struct epoll_event *evlist, int maxevents, int timeout);

/* pthread functions */
int Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);

int Pthread_join(pthread_t thread, void **retval);

int Pthread_detach(pthread_t thread);

pthread_t Pthread_self(void);

void Pthread_exit(void *retval);

int Pthread_mutex_lock(pthread_mutex_t *mutex);

int Pthread_mutex_unlock(pthread_mutex_t *mutex);

/*MIME type utility functions*/
char *tolowerStr(char *str);

char *getMimeType(char *filename);

char *getTimestamp();
int Shmget(key_t __key, size_t __size, int __shmflg);
void *Shmat(int __shmid, const void *__shmaddr, int __shmflg);
int Shmctl(int __shmid, int __cmd, struct shmid_ds *__buf);
ssize_t
sock_fd_read(int sock, void *buf, ssize_t bufsize, int *fd);
ssize_t sock_fd_write(int sock, void *buf, ssize_t buflen, int fd);
int is_cgi(char *uri, char *filename, char *cgiargs);
#endif