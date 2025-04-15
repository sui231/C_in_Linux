#include <func231.h>
#include "threadPool.h"

int exitPipe[2];

void sigFunc(int signum){
    printf("signum = %d\n", signum);
    write(exitPipe[1],"1",1);
    printf("Parent process is going to die!\n");
}

int main(int argc, char *argv[]){
    //./server 192.168.14.9 1234 3
    ARGS_CHECK(argc,4);
    pipe(exitPipe);

    if(fork() != 0){//父进程 先注册一个信号，等待子进程终止，最后自己终止
        close(exitPipe[0]);
        signal(SIGUSR1,sigFunc);
        wait(NULL);
        exit(0);
    }

    close(exitPipe[1]);

    int workerNum = atoi(argv[3]);
    threadPool_t threadPool;//为线程池的任务队列、子线程的tid申请内存
    threadPoolInit(&threadPool,workerNum);//初始化内存
    makeWorker(&threadPool);//创建若干个子线程
    
    int sockFd;
    tcpInit(&sockFd, argv[1], argv[2]);//主线程要初始化TCP连接
    int epfd = epoll_create(1);
    epollAdd(sockFd, epfd);//用epoll把sockfd监听起来
    epollAdd(exitPipe[0], epfd);
    struct epoll_event readyArr[2];

    while(1){
        printf("epoll_wait\n");
        int readyNum = epoll_wait(epfd, readyArr,2,-1);
        for(int i = 0; i < readyNum; ++i){
            if(readyArr[i].data.fd == sockFd){
                // 新客户端连接
                int netFd = accept(sockFd, NULL, NULL);
                //先加锁
                pthread_mutex_lock(&threadPool.taskQueue.mutex);
                //生产一个任务
                taskEnqueue(&threadPool.taskQueue, netFd);
                printf("New task!\n");
                
                //通知处于唤醒队列的线程
                pthread_cond_signal(&threadPool.taskQueue.cond);
                pthread_mutex_unlock(&threadPool.taskQueue.mutex);
            }
            else if(readyArr[i].data.fd == exitPipe[0]){
                printf("child process, threadPool is going to die\n");
                //for(int j = 0;j < workerNum; ++j){
                //    pthread_cancel(threadPool.tid[j]);
                //}
                threadPool.exitFlag = 1;
                pthread_cond_broadcast(&threadPool.taskQueue.cond);
                for(int j = 0; j < workerNum; ++j){
                    pthread_join(threadPool.tid[j], NULL);
                }
                pthread_exit(NULL);
            }
        }
    } 
    
}

int threadPoolInit(threadPool_t *pThreadPool, int workerNum){
    pThreadPool->threadNum = workerNum;
    pThreadPool->tid = (pthread_t *)calloc(workerNum,sizeof(pthread_t));
    pThreadPool->taskQueue.pFront = NULL;
    pThreadPool->taskQueue.pRear = NULL;
    pThreadPool->taskQueue.size = 0;

    pthread_mutex_init(&pThreadPool->taskQueue.mutex,NULL);
    pthread_cond_init(&pThreadPool->taskQueue.cond,NULL);
    
    pThreadPool->exitFlag = 0;
}