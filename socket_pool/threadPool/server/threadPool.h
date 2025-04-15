#include <func231.h>

// 用于构建任务链表
typedef struct task_s{
    int netFd;//传递文件描述符
    struct task_s *pNext;
} task_t;

// 任务队列结构体
typedef struct taskQueue_s{
    task_t *pFront; //队首指针
    task_t *pRear;  //队尾指针
    int size;       //队列现在的长度
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} taskQueue_t;

// 线程池结构体
typedef struct threadPool_s {
    pthread_t *tid;     //子线程的数组
    int threadNum;      //子线程的数量
    taskQueue_t taskQueue;
    int exitFlag;
} threadPool_t;

// 启动线程池的函数
int threadPoolInit(threadPool_t *pThreadPool, int workerNum);

int taskEnqueue(taskQueue_t *pTaskQueue, int netFd);

int taskDequeue(taskQueue_t *pTaskQueue);



int makeWorker(threadPool_t *pThreadPool);

int tcpInit(int *pSockFd, char *ip, char *port);

int epollAdd(int fd, int epfd);

int epollDel(int fd, int epfd);

int transFile(int netFd);

typedef struct train_s{
    int length;
    char buf[1000];
} train_t;