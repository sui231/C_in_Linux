#include <func231.h>
#define READYARR_SIZE 3

int main(int argc, const char* argv[]) {
    // ./server_reconnect 192.168.152.129 1234
    ARGC_CHECK(argc, 3);

    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(sockFd, -1, "socket");

    // SO_REUSEADDR
    int opt = 1;
    int ret = setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    ERROR_CHECK(ret, -1, "setsockopt");

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    ret = bind(sockFd, (struct sockaddr*)&addr, sizeof(addr));
    ERROR_CHECK(ret, -1, "bind");

    ret = listen(sockFd, 10);
    ERROR_CHECK(ret, -1, "listen");


    // epoll
    int epFd = epoll_create(1);
    ERROR_CHECK(epFd, -1, "epoll_create");

    struct epoll_event event;
    event.data.fd = STDIN_FILENO;
    event.events = EPOLLIN;
    ret = epoll_ctl(epFd, EPOLL_CTL_ADD, STDIN_FILENO, &event);
    ERROR_CHECK(ret, -1, "epoll_ctl");

    event.data.fd = sockFd;
    event.events = EPOLLIN;
    ret = epoll_ctl(epFd, EPOLL_CTL_ADD, sockFd, &event);
    ERROR_CHECK(ret, -1, "epoll_ctl");


    char buf[4096] = {0};
    struct epoll_event readyArr[READYARR_SIZE];
    int flag = 1;
    int netFd = -1;
    int isConnected = 0;
    time_t tmsg, tcheck;
    while(flag){
        int readyNum = epoll_wait(epFd, readyArr, READYARR_SIZE, 1000);
        puts("epoll wait");
        // 超时
        if(readyNum == 0 && isConnected != 0){
            puts("time out");
            tcheck = time(NULL);
            if((tcheck - tmsg > 5)){
                const char* msg = "time out disconnect";
                send(netFd, msg, strlen(msg), 0);
                close(netFd);
                event.data.fd = netFd;
                event.events = EPOLLIN;
                epoll_ctl(epFd, EPOLL_CTL_DEL, netFd, &event);
                isConnected = 0;
            }
        }

        for (int i = 0; i < readyNum; i++){
            if(readyArr[i].data.fd == sockFd){
                netFd = accept(sockFd, NULL, NULL);
                ERROR_CHECK(netFd, -1, "accept");

                event.data.fd = netFd;
                event.events = EPOLLIN;
                epoll_ctl(epFd, EPOLL_CTL_ADD, netFd, &event);
                isConnected = 1;
                printf("connect\n");
                tmsg = time(NULL);
            }
            else if(readyArr[i].data.fd == STDIN_FILENO){
                bzero(buf, sizeof(buf));
                ret = read(STDIN_FILENO, buf, sizeof(buf));
                if(isConnected == 0){
                    break;
                }
                if(ret == 0){
                    flag = 0;
                }
                send(netFd, buf, strlen(buf), 0);
            }
            else if(readyArr[i].data.fd == netFd){
                if(isConnected != 0){ // 
                    int noFd = accept(sockFd, NULL, NULL);
                    close(noFd);
                    continue;
                }
                bzero(buf, sizeof(buf));
                ret = recv(netFd, buf, sizeof(buf), 0);
                if(ret == 0){
                    isConnected = 0;
                    close(netFd);
                    event.data.fd = netFd;
                    event.events = EPOLLIN;
                    epoll_ctl(epFd, EPOLL_CTL_DEL, netFd, &event);
                }
                printf("(server recv)\n%s\n\n", buf);
                tmsg = time(NULL);
            }
        } // end for
    } // end while

    close(netFd);
    close(sockFd);
    close(epFd);
    return 0;
}