#include "acccement.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<sys/dir.h>
#include<dirent.h>


char* client_selectfile(int fd)
{
     accement recv_buf;
    static char filelist[100][20]={0};
    memset(filelist, 0, sizeof(filelist));
    int buf_count = 0;
    while (recv(fd, &recv_buf, sizeof(recv_buf), 0)) {
        if (recv_buf.code==ENDL) {
            break;
        }
        strncpy(filelist[buf_count], recv_buf.bitstring, sizeof(filelist)-1);
        printf("%d.%s\n",recv_buf.index,recv_buf.bitstring);
        buf_count++;
    }
    int index=0;
    printf("please input the index of file you want to download:\n");
    scanf("%d", &index);
    accement acm;
    acm.code=DATA;
    acm.cmd=directorytransmission;
    acm.index=index;
    send(fd, &acm, sizeof(acm), 0);
        printf("select file name:%s\n", filelist[index]);
return filelist[index];
}



int client_recvfile(int fd, char* filename)
{
    accement recv_buf;
    char filebuf[20]={0};

    printf("recv file name:%s\n", filename);
    int filefd=open(filename, O_RDWR|O_CREAT, 0777);
    if (filefd==-1) {
        perror("open file error");
        return -1;
    }
    
    recv(fd, &recv_buf, sizeof(recv_buf), 0);
    if (recv_buf.code!=STRT) {
        printf("file recv error\n");
        return -1;
    }
    ssize_t file_size=0;
    puts("file open success,files recv...\n");
    lseek(filefd, 0, SEEK_SET);
    while ((file_size=recv(fd, &recv_buf, sizeof(recv_buf), 0)) > 0) {
        if (recv_buf.code==ENDL) {
            break;
        }
        printf("本次接收%ld：%s\n",file_size, recv_buf.bitstring);
        write(filefd, recv_buf.bitstring, strlen(recv_buf.bitstring));
    }
    printf("file recv success\n");
    close(filefd);
    return 0;
}