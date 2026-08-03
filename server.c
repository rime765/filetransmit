#include "acccement.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#include<sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include<stdio.h>
#include <signal.h>
pthread_mutex_t mutex;
pthread_cond_t cond;


convstion conver[10];

void* thread_recv(void* sock);
void* thread_send(void* sock);
void sig_handler(int signo){
	if (signo == SIGTSTP) {
		printf("received SIGTSTP\n");
		for (int i=0; i<10; i++) {
			pthread_cancel(conver[i].recieve);
			pthread_cancel(conver[i].send);
			close(conver[i].fd);
		}
	}
}
int main()
{
	pthread_mutex_init(&mutex, 0);
	pthread_cond_init(&cond, 0);

	signal(SIGTSTP, sig_handler);

	int sock_cnet=socket(AF_INET,SOCK_STREAM ,0);
	if (sock_cnet==-1) {
		perror("套接字创建失败");
		return -1;
	}
	printf("套接字创建成功\n");

	struct sockaddr_in sockaddr_ln;

	sockaddr_ln.sin_port=htons(8899);
	sockaddr_ln.sin_family=AF_INET;
	sockaddr_ln.sin_addr.s_addr=inet_addr("192.168.158.129");


	if (bind(sock_cnet, (struct sockaddr*)&sockaddr_ln,sizeof(sockaddr_ln))==-1) {
	perror("套接字绑定失败");
	return -1;
	}
	puts("套接字绑定成功\n");

	if (listen(sock_cnet, 10)==-1) {
	perror("监听失败");
	return -1;
	}
	puts("监听成功\n");

	struct sockaddr addr_oppost;
	socklen_t socketlen=sizeof(addr_oppost);

	for (int i=0; i<10; ) {
		conver[i].fd=accept(sock_cnet, (struct sockaddr *)&addr_oppost, &socketlen);
		if (conver[i].fd!=-1) {
 /*p*/      pthread_mutex_lock(&mutex);
 
			conver[i].ipaddr=addr_oppost;
			
			pthread_create(&conver[i].send   , NULL, thread_send,&conver[i]);
			pthread_create(&conver[i].recieve, NULL, thread_recv,&conver[i]);

			puts("thread create success\n");
			sleep(3);
			
/*v*/		pthread_mutex_unlock(&mutex);
			pthread_cond_signal(&cond);
			sleep(1);
			pthread_cond_signal(&cond);

			i++;
		}
	}

	for (int i=0; i<10; i++) {
		pthread_join(conver[i].recieve, NULL);
		pthread_join(conver[i].send   , NULL);
	}

	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex);

}


void* thread_recv(void* sock)
{
	
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_cond_wait(&cond, &mutex);//等待主线程放好数据发送广播
	printf("thread_recv: prepared\n");
	//取所需数据
	pthread_mutex_lock(&mutex);
	convstion convs_ifo=*(convstion*)sock;
	pthread_mutex_unlock(&mutex);

	
}
void* thread_send(void* sock)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	
	usleep(100);

	printf("thread_send: prepared\n");
	
	pthread_mutex_lock(&mutex);
	convstion convs_ifo=*(convstion*)sock;
	pthread_mutex_unlock(&mutex);

	pthread_cond_signal(&cond);//通知主线程数据已取完

	//文件准备
	char filename[20]={0};
	while(1){
		strncpy(filename, server_getfilename(convs_ifo.fd), sizeof(filename)-1);
		if (filename[0]==0) {
			break;
		}
		filesend(convs_ifo.fd, filename);
		memset(filename, 0, sizeof(filename));
	};
	return NULL;
}

