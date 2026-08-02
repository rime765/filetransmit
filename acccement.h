#ifndef __ACCEMENT
#define __ACCEMENT
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>

typedef struct{
    pthread_t recieve;
    pthread_t send;
    struct sockaddr  ipaddr;
    int fd ;
} convstion;

#endif