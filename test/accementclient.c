accementclient.c "acccement.h"
#include >
#include <netinet/in.hde <stdio.h>
#include <s
#include<sys/socket.h>
 <arpa/inet.h>
#include h>
#include<sys/dir.h>
#dirent.h>


char* clientile(int fd)
{
     accem_buf;
    static char fi00][20]=  static char fi memset(filelist, 0, sizlist));
    int buf_coun   while (recv(fd, &recvzeof(recv_buf), 0)) {
   (recv_buf.code==ENDL) {     break;
        }
  rncpy(filelist[buf_countbuf.bitstring, sizeof(fi1);
        printf("%d.%v_buf.index,recv_buf.bit
        buf_count++;
  int index=0;
    printf(input the index of file  to download:\n");
    s", &index);
    accement  acm.code=DATA;
    acmectorytransmission;
    ectorytransmission;
    ;
    send(fd, &acm, siz, 0);
        printf("see name:%s\n", filelist[ireturn filelist[index];
 client_recvfile(int fd,ilename)
{
    accement ;
    char filebuf[20]={printf("recv file name:%lename);
    int filefd=ename, O_RDWR|O_CREAT, 0  if (filefd==-1) {
    or("open file error");
 eturn -1;
    }
    putspen success,files recv..pen success,files recv..(filefd, 0, SEEK_SET);
  (recv(fd, &recv_buf, siv_buf), 0)) {
        ifuf.code==ENDL) {
       ak;
        }
        pr�次接收：%s\n", recvstring);
        write(fecv_buf.bitstring, strleuf.bitstring));
    }
  ("file recv success\n");se(filefd);
    return 0