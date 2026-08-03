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

/*在start时发送实际发送的文件名，
以特定长度发送文件内容
最后发送end*/
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
    strncpy(acm.bitstring, filename, sizeof(acm.bitstring)-1);
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

//client请求拉取目录后，服务端将会发送目录下的所有文件，然后client将要接收的文件名对应的index发送给服务端，服务端将会发送该文件
/*逻辑：服务端发送列表，等待client选择，然后根据recv_buf.index返回对应的文件名*/
char* server_getfilename(int fd)
{
    char dirname[100]="./";
    DIR* dir_st=opendir(dirname);
    struct dirent* entry;
    static char filelist[100][20]={0};
    
    accement recv_buf;
    accement acm;
    acm.code=0;
    acm.cmd=directorytransmission;
    acm.code=DATA;
    
    for (int i=0; ; i++) {
        entry=readdir(dir_st);
        acm.index=i;
        if (entry==NULL) {
            break;
        }
        if (entry->d_type==DT_REG) {
            strncpy(filelist[i], entry->d_name, sizeof(filelist[i])-1);
            printf("%d: %s\n", i, filelist[i]);
            strncpy(acm.bitstring, filelist[i], sizeof(acm.bitstring)-1);
            send(fd, &acm, sizeof(acm), 0);
        }
    }
    acm.code=ENDL;
    send(fd, &acm, sizeof(acm), 0);
    recv(fd, &recv_buf, sizeof(recv_buf), 0);
    return filelist[recv_buf.index]; //返回指定文件名，实际应用中可以根据需要修改
    closedir(dir_st);
    return NULL;
}
/*
接收主机发送的文件列表
输入序号，选择你要发送的文件
将序号信息发送给主机，主机将会发送该文件
*/
