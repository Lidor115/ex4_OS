/*
 * Lidor Alis
 * 201025160
 * Group: 06
 */

#include <unistd.h>
#include "threadPool.h"
#include <string.h>

#define  ERROR "Error in system call\n"

/**
 *write error end exit (-1)
 * @param threadpool - the threadpool
 */
void writeError(ThreadPool *threadpool) {
    write(2, ERROR, strlen(ERROR));
    // free all the memory that allocated
    freeAllThreadPool(threadpool);
    exit(-1);
}

/**
 * free all the allocating space.
 * @param threadpool - the threadpool
 */
void freeAllThreadPool(ThreadPool *threadpool) {
    // free the tasks and the Queue
    myTask *task;
    while (!osIsQueueEmpty(threadpool->osQueue)) {
        task = osDequeue(threadpool->osQueue);
        if (task != NULL) {
            free(task);
            task = NULL;
        }
    }
    osDestroyQueue(threadpool->osQueue);
    threadpool->osQueue = NULL;
    // free the array of the threads
    int i;
    if (threadpool->threadArray != NULL) {
        free(threadpool->threadArray);
    }
    if (pthread_mutex_lock(&threadpool->mutex) != 0) {
        write(2, ERROR, strlen(ERROR));
        free(threadpool);
        threadpool = NULL;
        exit(-1);
    }
    pthread_mutex_destroy(&threadpool->mutex);
    pthread_cond_destroy(&threadpool->fill);
    threadpool->status = DONE;
    // free the ThreadPool
    if (threadpool != NULL) {
        free(threadpool);
    }
}

/**
 * the function that runs on the threads, checking the status and if its ok
 * execute the function
 * @param func (this is the threadpool)
 */
void *threadFunc(void *func) {
    myTask *task;
    ThreadPool *threadpool = (ThreadPool *) func;

// while the status is not Done!!!
    while (threadpool->status != DONE) {
        // unlock
        if (pthread_mutex_unlock(&threadpool->mutex) != 0) {
            writeError(threadpool);
        }
        // if the status is destroy (0) - break. if the status is wait and the queue is empty - break
        if (threadpool->status == DESTROY || (threadpool->status == WAIT && osIsQueueEmpty(threadpool->osQueue))) {
            if (pthread_mutex_unlock(&threadpool->mutex) != 0) {
                writeError(threadpool);
            }
            break;
        }
        // lock
        if (pthread_mutex_lock(&threadpool->mutex) != 0) {
            writeError(threadpool);
        }
        // if "NORMAL" and the queue is empty - wait for another tasks
        while (osIsQueueEmpty(threadpool->osQueue) && threadpool->status == NORMAL) {
            pthread_cond_wait(&threadpool->fill, &threadpool->mutex);
        }
        // if the queue is empty - unlock and continue
        if (osIsQueueEmpty(threadpool->osQueue)) {
            if (pthread_mutex_unlock(&threadpool->mutex) != 0) {
                writeError(threadpool);
            }
            continue;
        }
        //make another task and if its not null - unlock, run and free the task!
        task = osDequeue(threadpool->osQueue);
        if (task != NULL) {
            if (pthread_mutex_unlock(&threadpool->mutex) != 0) {
                writeError(threadpool);
            }
            ((task->computeFunc))(task->param);
            free(task);
            task = NULL;
        }
    }
}

/**
 * initialize the array of the threads
 * @param num - the number of threads (from user)
 * @param threadPool the threadpool
 */
void initThreadArray(int num, ThreadPool *threadPool) {
    threadPool->threadArray = (pthread_t *) malloc(num * (sizeof(pthread_t)));
    if (threadPool->threadArray == NULL) {
        writeError(threadPool);
    }

    int i;
    // create the pthreads in the number that the user give.
    for (i = 0; i < num; ++i) {
        int x = pthread_create(&(threadPool->threadArray[i]), NULL, threadFunc, (void *) threadPool);
        if (x != 0) {
            writeError(threadPool);
        }
    }
}

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param) {
    // if its after tpDetroy - Don't insert
    if (threadPool->status == NORMAL) {
        //lock
        if (pthread_mutex_lock(&threadPool->mutex) != 0) {
            writeError(threadPool);
        }
        // allocate space for new task
        myTask *task = (myTask *) malloc(sizeof(myTask));
        if (task == NULL) {
            writeError(threadPool);
        }
        //fill the parameters of the task to run the task in the thread
        task->computeFunc = computeFunc;
        task->param = param;
        //Enqueue the task into the queue
        osEnqueue(threadPool->osQueue, task);
        //give a signal to the condition
        pthread_cond_signal(&threadPool->fill);
        // unlock
        pthread_mutex_unlock(&threadPool->mutex);
        return 0;
    }
    // else (not normal - return  1)
    return -1;
}

/**
 * create new ThreadPool (queue, list of pthreads) and set the status to normal
 * @param numOfThreads - the number of threads (from the user)
 * @return the threadpool
 */
ThreadPool *tpCreate(int numOfThreads) {
    // allocate space for the threadpool
    ThreadPool *threadPool = (ThreadPool *) malloc(sizeof(ThreadPool));
    if (threadPool == NULL) {
        writeError(threadPool);
    }
    // create the new Queue
    threadPool->osQueue = osCreateQueue(threadPool->osQueue);
    threadPool->numOfThreads = numOfThreads;
    // initialize the mutex and the fill condition
    pthread_mutex_init(&threadPool->mutex, NULL);
    pthread_cond_init(&threadPool->fill, NULL);
    // set the status to normal
    threadPool->status = NORMAL;
    // initializr the array
    initThreadArray(numOfThreads, threadPool);
    return threadPool;
}

/**
 * destroy the threadpool - if 0 - not to execute the tasks in the queue (DESTROY) and
 * else - wait that the tasks will finish (WAIT). in every case we will not get more tasks
 * @param threadPool
 * @param shouldWaitForTasks
 */
void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks) {
    if (threadPool->status != NORMAL) {
        if (pthread_mutex_unlock(&threadPool->mutex) != 0) {
            writeError(threadPool);
        }
        return;
    }
    // switch - case that set if to wait or not
    switch (shouldWaitForTasks) {
        case 0:
            threadPool->status = DESTROY;
            break;
        default:
            threadPool->status = WAIT;
            break;
    }
    // send to all the threads the changes
    pthread_cond_broadcast(&threadPool->fill);
    int i;
    //join all the threads
    for (i = 0; i < threadPool->numOfThreads; ++i) {
        pthread_join(threadPool->threadArray[i], NULL);
    }
    freeAllThreadPool(threadPool);
    threadPool = NULL;
}
