#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include<sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "acccement.h"
#include <sys/fcntl.h>
#include <signal.h>



pthread_mutex_t mutex;
pthread_cond_t cond;


convstion conver;

void sig_handler(int signo)
{
	if (signo == SIGTSTP) {
		printf("received SIGTSTP\n");
		pthread_cancel(conver.recieve);
		pthread_cancel(conver.send);
		close(conver.fd);
		exit(0);
	}
}

void* thread_recv(void* sock);
void* thread_send(void* sock);


int main()
{
	pthread_mutex_init(&mutex, 0);
	pthread_cond_init(&cond, 0);

	signal(SIGTSTP, sig_handler);
	
	int sock=socket(AF_INET, SOCK_STREAM, 0);
	if(sock==-1){
		perror("socket cr mistake");
		return -1;
	}
	printf("socket create success\n");

	struct sockaddr_in ipaddr;
	ipaddr.sin_addr.s_addr=inet_addr("192.168.158.129");
	ipaddr.sin_port=htons(8899);
	ipaddr.sin_family=AF_INET;

	int cnnt=
	connect(sock, (struct sockaddr*)&ipaddr  , sizeof(ipaddr ));
	if(cnnt==-1){
		perror("connct mistake");
		return -1;
	}
	puts("connect success\n");
	
	pthread_mutex_lock(&mutex);

	conver.fd=sock;
	
	pthread_create(&conver.recieve, NULL, thread_recv,&conver);
	pthread_create(&conver.send   , NULL, thread_send,&conver);

	pthread_mutex_unlock(&mutex);



	pthread_join(conver.recieve, NULL);
	pthread_join(conver.send, NULL);
	
	sleep(10);
	pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
	
	while(1);
}

void* thread_recv(void* sock)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);


	printf("thread_recv: prepared\n");
	//取所需数据

	pthread_mutex_lock(&mutex);
    convstion convs_ifo=*(convstion*)sock;
    pthread_mutex_unlock(&mutex);
	pthread_cond_signal(&cond);

	char cvbuf[5]={0};
	char filename[5]={0};
	int fd=0;
	usleep(1000);
	while (1) {
		recv(convs_ifo.fd, cvbuf, sizeof(cvbuf)-1,0);//接收代码存在问题
		printf("recv:        %s\n",cvbuf);
		if (strcmp(cvbuf, "strt")==0) {
			memset(cvbuf, 0, sizeof(filename));

			recv(convs_ifo.fd, filename, sizeof(cvbuf)-1,0);
			printf("filename: %s\n", filename);
			if((fd=open(filename, O_RDWR|O_CREAT,0777))!=-1)
			{
				memset(filename, 0, sizeof(filename));
				lseek(fd, 0, SEEK_SET);
				continue;
			}
			else {
				perror("open erro");
				pthread_cancel(pthread_self());
			}
		}
		else if (strcmp(cvbuf, "endl")==0) {
			memset(cvbuf, 0, sizeof(filename));
			close(fd);
			break;
		}
		else {
			write(fd, cvbuf	,strlen(cvbuf));
			memset(cvbuf, 0, sizeof(filename));
		}
	}

	return NULL;
}


void* thread_send(void* sock)
{

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_cond_wait(&cond, &mutex);
	
	sleep(1);
    printf("thread_send: prepared\n");

    pthread_mutex_lock(&mutex);
    convstion convs_ifo=*(convstion*)sock;
    pthread_mutex_unlock(&mutex);

	char sdbuf[10]={0};
	scanf("%s",sdbuf);
	send(convs_ifo.fd, sdbuf, strlen(sdbuf), 0);


    while(1);
    return NULL;
}