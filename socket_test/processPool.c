#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/un.h>
#include <fcntl.h>

// 自定义错误检查宏
#define ARGS_CHECK(argc, num) \
    if (argc != num) { \
        printf("Usage: %s <ip> <port> <worker_num>\n", argv[0]); \
        exit(1); \
    }

#define ERROR_CHECK(ret, err_val, msg) \
    if (ret == err_val) { \
        perror(msg); \
        exit(1); \
    }

// 定义存储工作进程信息的结构体
typedef struct {
    pid_t pid;  // 进程 ID
    int status; // 进程状态，FREE 或 BUSY
    int pipeFd; // 管道文件描述符
} processData_t;

#define FREE 0
#define BUSY 1

// 在管道中发送文件描述符
int sendFd(int sockfd, int fd) {
    struct msghdr msg;
    struct cmsghdr *cmsg;
    char buf[1] = {0};
    struct iovec iov = {buf, sizeof(buf)};

    union {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    } control_un;

    cmsg = (struct cmsghdr *)control_un.control;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    *(int *)CMSG_DATA(cmsg) = fd;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control_un.control;
    msg.msg_controllen = cmsg->cmsg_len;

    return sendmsg(sockfd, &msg, 0);
}

// 在管道中接收文件描述符
int recvFd(int sockfd, int *fd) {
    struct msghdr msg;
    struct cmsghdr *cmsg;
    char buf[1] = {0};
    struct iovec iov = {buf, sizeof(buf)};

    union {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    } control_un;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);

    int n = recvmsg(sockfd, &msg, 0);
    if (n < 0) {
        return -1;
    }

    cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg && cmsg->cmsg_len == CMSG_LEN(sizeof(int)) &&
        cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
        *fd = *(int *)CMSG_DATA(cmsg);
        return 0;
    }

    return -1;
}

// TCP 初始化相关代码
int tcpInit(char *ip, char *port, int *pSockFd) {
    *pSockFd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(atoi(port));
    int reuse = 1;
    int ret;
    ret = setsockopt(*pSockFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    ERROR_CHECK(ret, -1, "setsockopt");
    ret = bind(*pSockFd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    ERROR_CHECK(ret, -1, "bind");
    listen(*pSockFd, 10);
    return 0;
}

// epoll 相关代码
int epollCtor() {
    int epfd = epoll_create(1);
    ERROR_CHECK(epfd, -1, "epoll_create");
    return epfd;
}

int epollAdd(int fd, int epfd) {
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
    ERROR_CHECK(ret, -1, "epoll_ctl add");
    return 0;
}

int epollDel(int fd, int epfd) {
    int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    ERROR_CHECK(ret, -1, "epoll_ctl del");
    return 0;
}

// 创建子进程
int makeChild(processData_t *pProcssData, int processNum) {
    pid_t pid;
    for (int i = 0; i < processNum; ++i) {
        int pipeFd[2];
        socketpair(AF_LOCAL, SOCK_STREAM, 0, pipeFd);
        pid = fork();
        if (pid == 0) {
            close(pipeFd[0]);
            handleEvent(pipeFd[1]);
        }
        close(pipeFd[1]);
        printf("pid = %d, pipefd[0] = %d\n", pid, pipeFd[0]);
        pProcssData[i].pid = pid;
        pProcssData[i].status = FREE;
        pProcssData[i].pipeFd = pipeFd[0];
    }
    return 0;
}

// 子进程处理事件
void handleEvent(int pipeFd) {
    int netFd;
    while (1) {
        recvFd(pipeFd, &netFd);
        char buf[1024] = {0};
        recv(netFd, buf, sizeof(buf), 0);
        puts(buf);
        send(netFd, "Echo", 4, 0);
        close(netFd);
        pid_t pid = getpid();
        send(pipeFd, &pid, sizeof(pid_t), 0);
    }
}

// 服务端主进程
int main(int argc, char *argv) {
    //./main 192.168.135.132 5678 10
    ARGS_CHECK(argc, 4);
    int workerNum = atoi(argv[3]);
    processData_t *workerList = (processData_t *)calloc(sizeof(processData_t), workerNum);
    makeChild(workerList, workerNum);
    int sockFd;
    tcpInit(argv[1], argv[2], &sockFd);
    int epfd = epollCtor();
    epollAdd(sockFd, epfd);
    for (int i = 0; i < workerNum; ++i) {
        epollAdd(workerList[i].pipeFd, epfd);
    }
    int listenSize = workerNum + 1; // socket + 每个进程 pipe 的读端
    struct epoll_event *readylist = (struct epoll_event *)calloc(listenSize, sizeof(struct epoll_event));
    while (1) {
        int readynum = epoll_wait(epfd, readylist, listenSize, -1);
        for (int i = 0; i < readynum; ++i) {
            if (readylist[i].data.fd == sockFd) {
                puts("accept ready");
                int netFd = accept(sockFd, NULL, NULL);
                for (int j = 0; j < workerNum; ++j) {
                    if (workerList[j].status == FREE) {
                        printf("No. %d worker gets his job, pid = %d\n", j, workerList[j].pid);
                        sendFd(workerList[j].pipeFd, netFd);
                        workerList[j].status = BUSY;
                        break;
                    }
                }
                close(netFd); // 父进程交给子进程一定要关闭
            } else {
                puts("One worker finish his task!");
                int j;
                for (j = 0; j < workerNum; ++j) {
                    if (workerList[j].pipeFd == readylist[i].data.fd) {
                        pid_t pid;
                        int ret = recv(workerList[j].pipeFd, &pid, sizeof(pid_t), 0);
                        printf("No. %d worker finish, pid = %d\n", j, pid);
                        workerList[j].status = FREE;
                        break;
                    }
                }
            }
        }
    }
}

// 客户端
int main_client(int argc, char *argv[]) {
    ARGS_CHECK(argc, 3);
    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));
    int ret = connect(sockFd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    ERROR_CHECK(ret, -1, "connect");
    char buf[1024] = {0};
    read(STDIN_FILENO, buf, sizeof(buf));
    send(sockFd, buf, strlen(buf) - 1, 0);
    bzero(buf, sizeof(buf));
    recv(sockFd, buf, sizeof(buf), 0);
    puts(buf);
    close(sockFd);
    return 0;
}