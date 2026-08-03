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


//协议
//接收者判断command决定是直接打印还是写入文件
typedef enum{
    STRT,
    ENDL,
    DATA
} filecode;

typedef enum{
    conversation,
    filetransmission,
    cdirectorytransmission
}command;

typedef struct{
    filecode code;
    command cmd;
    char sendbuf[20];
} accement;



int filesend(int fd, char* filename);
#endif