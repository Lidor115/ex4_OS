#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <sys/types.h>
#include "osqueue.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*
*/

typedef struct {
    void (*computeFunc)(void *);

    void *param;
} myTask;

typedef struct thread_pool {
    int isAlive;
    pthread_cond_t empty, fill;
    pthread_mutex_t mutex;
    OSQueue *osQueue;
    pthread_t *threadArray;
    //TODO - FILL THIS WITH YOUR FIELDS
} ThreadPool;

void *threadFunc(ThreadPool *threadPool);

void initThredArray(int num, ThreadPool *threadPool);

ThreadPool *tpCreate(int numOfThreads);

void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param);

#endif
