#include "worker.h"
#include "head.h"
int makeChild(workerData_t *workerArr, int workerNum){
    pid_t pid;
    int pipeFd[2];
    for(int i = 0; i < workerNum; ++i){
        socketpair(AF_LOCAL,SOCK_STREAM,0,pipeFd);
        pid = fork();
        if(pid == 0){
            //子进程进入子进程
            close(pipeFd[0]);
            eventHanler(pipeFd[1]);
        }
        //父进程
        close(pipeFd[1]);
        printf("pipeFd = %d, pid = %d\n", pipeFd[0], pid);
        workerArr[i].pipeFd = pipeFd[0];
        workerArr[i].pid = pid;
        workerArr[i].status = FREE;
    }
}

int eventHanler(int pipeFd){
    while(1){
        int netFd;
        int exitFlag;

        recvFd(pipeFd, &netFd, &exitFlag);
        if(exitFlag == 1){
            puts("I am going to die!\n");
            exit(0);
        }

        //后续的任务加在这里
        printf("I got a task!\n");
        transFile(netFd);
        printf("I have done this task!\n");
        pid_t pid = getpid();
        write(pipeFd,&pid,sizeof(pid));

        close(netFd);
    }
}
