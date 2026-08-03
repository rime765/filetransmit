#include "acccement.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int filesend(int fd, char* filename)
{
    accement acm;
    acm.code=STRT;
    acm.cmd=filetransmission;

    char sendbuf[20]={0};

    int filefd=open(filename, O_RDONLY);
    if (filefd==-1) {
        perror("open file error");
        return -1;
    }

    //发送文件名/起始
    printf("file open success,files send...\n");
    strncpy(acm.sendbuf, filename, sizeof(acm.sendbuf)-1);
    send(fd, &acm, sizeof(acm), 0);

    
    while (read(filefd, sendbuf, sizeof(sendbuf)-1)>0) {
        send(fd, sendbuf, strlen(sendbuf), 0);
        memset(sendbuf, 0, sizeof(sendbuf));
    }
    close(filefd);

    acm.code=ENDL;
    send(fd, &acm, sizeof(acm), 0);

    return 0;
}