#include <func231.h>
#include "head.h"
#include "worker.h"

int exitPipe[2];

void sigFunc(int signum){
    printf("signum = %d\n", signum);
    write(exitPipe[1],"1",1);
}

int main(int argc, char *argv[]){
    // ./server 192.168.4.19 1234 3
    // 创建很多子进程
    int workerNum = atoi(argv[3]);
    workerData_t * workerArr = (workerData_t *)calloc(workerNum, sizeof(workerData_t));

    makeChild(workerArr,workerNum);

    //父进程要注册信号
    signal(SIGUSR1, sigFunc);
    pipe(exitPipe);

    // 初始化tcp连接
    int sockFd;
    tcpInit(&sockFd, argv[1], argv[2]);
    // 用epoll把tcp连接和子进程管理起来
    int epfd = epoll_create(1);
    epollAdd(sockFd, epfd);
    epollAdd(exitPipe[0], epfd);
    for(int i = 0; i < workerNum; ++i){
        epollAdd(workerArr[i].pipeFd, epfd);
    }

    int listenSize = workerNum + 2;
    int exitFlag = 0;
    struct epoll_event *readyArr = (struct epoll_event *)calloc(listenSize, sizeof(struct epoll_event));
    
    while(1){
        int readyNum = epoll_wait(epfd,readyArr,listenSize,-1);
        printf("epoll_wait ready!\n");
        for(int i = 0; i < readyNum; ++i){

            if(readyArr[i].data.fd == sockFd){
                puts("client connect");
                int netFd = accept(sockFd,NULL,NULL);
                ERROR_CHECK(netFd,-1,"accept");
                // 把网络连接移交给子进程
                for(int j = 0; j < workerNum; ++j){
                    if(workerArr[j].status == FREE){
                        printf("%d worker got a job, pid = %d\n", j, workerArr[j].pid);
                        sendFd(workerArr[j].pipeFd,netFd,exitFlag);
                        close(netFd);
                        workerArr[j].status = BUSY;
                        break;
                    }
                }

            } else if(readyArr[i].data.fd == exitPipe[0]){
                printf("process pool is going to exit!\n");
                exitFlag = 1;
                for(int j = 0; j < workerNum; ++j){
                    //kill(workerArr[j].pid,SIGUSR1);
                    sendFd(workerArr[j].pipeFd,0,exitFlag);
                }
                for(int j = 0; j < workerNum; ++j){
                    wait(NULL);
                }
                printf("process pool exits!\n");
                exit(0);
            }
            else{
                puts("one worker finishes his job!\n");
                int j;
                for(j = 0; j < workerNum; ++j){
                    if(readyArr[i].data.fd == workerArr[j].pipeFd){
                        pid_t pid;
                        int ret = read(workerArr[j].pipeFd,&pid,sizeof(pid));
                        printf("%d worker finish his job, pid = %d\n", j, pid);
                        workerArr[j].status = FREE;
                        break;
                    }
                }
            }
        }
    }
}