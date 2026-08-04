#ifndef __PTP_H
#define __PTP_H
#include <pthread.h>
#include <stdbool.h>
typedef     void* (*func)(void*) ;

typedef struct _TST_NODE{
    void* arg;
    void* (*func)(void*);
    struct _TST_NODE* next;
} tast_t;

typedef struct _PTP{
    tast_t* head;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t tid[20];
    bool is_running;
    pthread_t start;
    pthread_t manager;
} ptp_t;

typedef struct{
    ptp_t pool;

} ptp_c;

#endif