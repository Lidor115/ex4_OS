//
// Created by ubuntu19 on 5/5/19.
//


#include "threadPool.h"

void *threadFunc(ThreadPool *threadPool) {
    while (threadPool->isAlive) {
        pthread_mutex_lock(&threadPool->mutex);
        while (!osIsQueueEmpty(threadPool->osQueue))

    }

}

void initThredArray(int num, ThreadPool *threadPool) {
    threadPool->threadArray = (pthread_t *) malloc(num * sizeof(pthread_t));
    if (threadPool->threadArray == NULL) {
        perror("fail in malloc threadArray");
        exit(1);
    }

    int i = 0;
    for (i = 0; i < num; ++i) {
        threadPool->threadArray[i] = pthread_create(&threadPool->threadArray[i], NULL, threadFunc(threadPool), NULL);
        if (threadPool->threadArray[i] != 0) {
            perror("pthread_create() error");
            exit(1);
        }
    }
}

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param) {
    pthread_mutex_lock(&threadPool->mutex);
    myTask *task = (myTask *) malloc(sizeof(task));
    if (task == NULL) {
        perror("fail in malloc task");
        exit(1);
    }
    task->computeFunc = computeFunc;
    task->param = param;
    osEnqueue(threadPool->osQueue, task);
    pthread_cond_signal(&threadPool->fill);
    pthread_mutex_unlock(&threadPool->mutex);
}

ThreadPool *tpCreate(int numOfThreads) {
    ThreadPool *threadPool = (ThreadPool *) malloc(sizeof(ThreadPool));
    if (threadPool == NULL) {
        perror("fail in malloc ThreadPool");
        exit(1);
    }
    threadPool->isAlive = true;
    initThredArray(numOfThreads, threadPool);
    return threadPool;
}

