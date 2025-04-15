#include "threadPool.h"

int makeWorker(threadPool_t *pThreadPool){
    for(int i = 0; i < pThreadPool->threadNum; ++i){
        pthread_create(&pThreadPool->tid[i], NULL, handleEvent, (void *)pThreadPool);
    }
}

void* handleEvent(void *arg){
    threadPool_t * pThreadPool = (threadPool_t *)arg;
    int netFd;
    while(1){
        printf("free, tid = %lu\n", pthread_self());
        pthread_mutex_lock(&pThreadPool->taskQueue.mutex);
        pthread_cleanup_push(cleanFunc,(void *)pThreadPool);
        while(pThreadPool->taskQueue.size == 0 && pThreadPool->exitFlag == 0){
            pthread_cond_wait(&pThreadPool->taskQueue.cond,&pThreadPool->taskQueue.mutex);
        }
        
        if(pThreadPool->exitFlag != 0){
            printf("child thread die, tid = %lu\n", pthread_self());
            pthread_exit(NULL);
        }

        // 子线程
        // 队首的文件描述符
        netFd = pThreadPool->taskQueue.pFront->netFd;
        taskDequeue(&pThreadPool->taskQueue);
        pthread_cleanup_pop(1);
        printf("child thread work, tid = %lu\n", pthread_self());
        transFile(netFd);
        printf("done\n");
        close(netFd);
    }
}

void cleanFunc(void *arg){
    threadPool_t * pThreadPool = (threadPool_t *)arg;
    pthread_mutex_unlock(&pThreadPool->taskQueue.mutex);
}

