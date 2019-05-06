//
// Created by ubuntu19 on 5/5/19.
//


#include "threadPool.h"

void *threadFunc(ThreadPool *threadPool) {
// is alive  = 1 - run, is alive  = 0 - don't run , is alive = 2 cant
    while (threadPool->isAlive == 1) {
        pthread_mutex_lock(&threadPool->mutex); // check if its here
        while (!osIsQueueEmpty(threadPool->osQueue) && threadPool->isAlive > 0) {
            if (threadPool->isAlive == 2) {
                pthread_mutex_unlock(&threadPool->mutex);
                break;
            }
            myTask *task = osDequeue(threadPool->osQueue);
            task->computeFunc(task->param);
            free(task);
            if (osIsQueueEmpty(threadPool->osQueue)) {
                pthread_cond_wait(&threadPool->fill, &threadPool->mutex);
            }
            pthread_mutex_unlock(&threadPool->mutex);

        }

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
    pthread_mutex_init(&threadPool->mutex, NULL);
    pthread_cond_init(&threadPool->fill, NULL);
    pthread_cond_init(&threadPool->empty, NULL);
    threadPool->isAlive = 1;
    initThredArray(numOfThreads, threadPool);
    return threadPool;
}

void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks) {
    if (shouldWaitForTasks == 0) {
        threadPool->isAlive = 0;
        while (!osIsQueueEmpty(threadPool->osQueue)) {
            myTask *task = osDequeue(threadPool->osQueue);
            free(task);
        }
    } else {
        threadPool->isAlive = 2;
    }
    //Todo check when to free the threads
    //Todo - free the queue

}
