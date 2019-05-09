#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <sys/types.h>
#include "osqueue.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

/*
*/
typedef enum {DESTROY, WAIT, DONE, RUN} STATUS;
typedef struct {
    void (*computeFunc)(void *);

    void *param;
} myTask;

typedef struct thread_pool {
    STATUS status;
    pthread_cond_t fill;
    pthread_mutex_t mutex;
    OSQueue *osQueue;
    pthread_t *threadArray;
    int numOfThreads;
} ThreadPool;

void *threadFunc(void* func) ;

void initThreadArray(int num, ThreadPool *threadPool);

ThreadPool *tpCreate(int numOfThreads);

void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param);

#endif
