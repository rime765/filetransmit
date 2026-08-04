client.c <fcntl.h>
#include <neth>
#include <pthread.h>
 <stdio.h>
#include <stdinclude <string.h>

#inc/socket.h>
#include <arp>
#include <unistd.h>
#iacccement.h"
#include <s.h>
#include <snclude <s



pthreade <snclude <s mutex;
pthread_cond_t c mutex;
pthread_cond_t con conver;

void sig_han signo)
{
	if (signo ==  {
		printf("received SI);
		pthread_cancel(convve);
		pthread_cancel(cod);
		close(conver.fd);
);
	}
}

void* thread_re sock);
void* thread_sensoock);
void* thread_sennt main()
{
	pthread_mut&mutex, 0);
	pthread_concond, 0);

	signal(SIGTShandler);
	
	int sock=soINET, SOCK_STREAM, 0);
	=-1){
		perror("socket ce");
		return -1;
	}
	prcket create success\n");t sockaddr_in ipaddr;
	in_addr.s_addr=inet_addr(.158.129");
	ipaddr.sin_ns(8899);
	ipaddr.sin_faINET;

	int cnnt=
	conne (struct sockaddr*)&ipadzeof(ipaddr ));
	if(cnnt	perror("connct mistake"urn -1;
	}
	puts("connecs\n");
	
	pthread_mutex_tex);

	conver.fd=sock;
ad_create(&conver.reciev thread_recv,&conver);
	create(&conver.send   , read_send,&conver);

	pttex_unlock(&mutex);



	joi_unlock(&mutex);



	.recieve, NULL);
	pthreaonver.send, NULL);
	
	sl
	pthread_cond_destroy(&   pthread_mutex_destroy;
	
	while(1);
}

void* ecv(void* sock)
{
    pttcancelstate(PTHREAD_CANLE, NULL);


	printf("thv: prepared\n");
	//取�据

	pthread_mutex_lock;
    convstion convs_ifstion*)sock;
    pthreadnlock(&mutex);
	pthread_nal(&cond);

	char cvbuf
	char filename[5]={0};
0;
	usleep(1000);
	while	client_recvfile(convs_ilient_selectfile(convs_i
	}

	return NULL;
}


vead_send(void* sock)
{

ead_setcancelstate(PTHREL_ENABLE, NULL);
    pthd_wait(&cond, &mutex);

tf("thread_send: prepare    pthread_mutex_lock(&    convstion convs_ifo=ion*)sock;
    pthread_mock(&mutex);

 	while(1)lient_recvfile(convs_ifoent_selectfile(convs_ifo1){
			puts("file recv e			break;
		}
		puts("fisuccess\n");
 	}
    ret;
}