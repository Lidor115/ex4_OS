//
// Created by ubuntu19 on 5/5/19.
//


#include <unistd.h>
#include "threadPool.h"
#include <string.h>

void *threadFunc(void *func) {
    ThreadPool *tp = (ThreadPool *) func;
// is alive  = 1 - run, is alive  = 0 - don't run , is alive = 2 can't insert new tasks
    while (tp->status != DONE) {
        pthread_mutex_lock(&tp->mutex);
        if (tp->status == DESTROY || (tp->status == WAIT && osIsQueueEmpty(tp->osQueue) == 1)) {
            if (pthread_mutex_unlock(&tp->mutex) != 0) {
                write(2, "Error", strlen(sizeof("Error")));
                exit(-1);
            }
            break;
        }
        if (pthread_mutex_unlock(&tp->mutex) != 0) {
            write(2, "Error", strlen(sizeof("Error")));
            exit(-1);
        }
        while (osIsQueueEmpty(tp->osQueue) && tp->status == RUN) {
            pthread_cond_wait(&tp->fill, &tp->mutex);
        }
        if (osIsQueueEmpty(tp->osQueue)) {
            if (pthread_mutex_unlock(&tp->mutex) != 0) {
                write(2, "Error", strlen(sizeof("Error")));
                exit(-1);
            }
            continue;
        }

        myTask *task = osDequeue(tp->osQueue);
        if (task != NULL) {
            ((task->computeFunc))(task->param);
            free(task);
        }
        pthread_cond_signal(&tp->fill);
        pthread_mutex_unlock(&tp->mutex);
    }
}


void initThreadArray(int num, ThreadPool *threadPool) {
    threadPool->threadArray = (pthread_t *) malloc(num * (sizeof(pthread_t)));
    if (threadPool->threadArray == NULL) {
        perror("fail in malloc threadArray");
        exit(1);
    }

    int i;
    for (i = 0; i < num; ++i) {
        int x = pthread_create(&(threadPool->threadArray[i]), NULL, threadFunc, (void *) threadPool);
        if (x != 0) {
            perror("pthread_create() error");
            exit(1);
        }
    }
}

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param) {
    if (threadPool->status == DONE || threadPool->status == WAIT) {
        return 0;
    }
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
    return 1;
}

ThreadPool *tpCreate(int numOfThreads) {
    ThreadPool *threadPool = (ThreadPool *) malloc(sizeof(ThreadPool));
    if (threadPool == NULL) {
        perror("fail in malloc ThreadPool");
        exit(1);
    }
    threadPool->osQueue = osCreateQueue(threadPool->osQueue);
    threadPool->numOfThreads = numOfThreads;
    pthread_mutex_init(&threadPool->mutex, NULL);
    pthread_cond_init(&threadPool->fill, NULL);
    threadPool->status = RUN;
    initThreadArray(numOfThreads, threadPool);
    return threadPool;
}

void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks) {

    if (shouldWaitForTasks == 0) {
        threadPool->status = DESTROY;
        while (!osIsQueueEmpty(threadPool->osQueue)) {
            myTask *task = osDequeue(threadPool->osQueue);
            free(task);
        }
    } else {
        threadPool->status = WAIT;
    }
    pthread_cond_broadcast(&threadPool->fill);
    int i;
    for (i = 0; i < threadPool->numOfThreads; ++i) {
        pthread_join(threadPool->threadArray[i], NULL);
    }
    pthread_mutex_destroy(&threadPool->mutex);
    pthread_cond_destroy(&threadPool->fill);
    free(threadPool->threadArray);
    osDestroyQueue(threadPool->osQueue);
    free(threadPool);
}
