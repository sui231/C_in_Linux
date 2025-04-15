#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

void printClient(int sockFd, struct sockaddr_in* client_addr, socklen_t* client_addr_len) {
    printf("sock_fd = %d\n", sockFd);
    printf("client_addr_len = %d\n", *client_addr_len);
    printf("client family = %d\n", client_addr->sin_family);
    printf("client port = %d\n", ntohs(client_addr->sin_port));
    printf("client address = %s\n\n", inet_ntoa(client_addr->sin_addr));
}

// 处理客户端断开连接
void handleClientDisconnect(int *netFdArr, int *curConn, int index) {
    close(netFdArr[index]);
    for (int j = index; j < *curConn - 1; j++) {
        netFdArr[j] = netFdArr[j + 1];
    }
    (*curConn)--;
}

// 支持断线重连
int main(int argc, const char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP address> <port>\n", argv[0]);
        return 1;
    }

    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd == -1) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(sockFd);
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    int ret = bind(sockFd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1) {
        perror("bind");
        close(sockFd);
        return 1;
    }

    ret = listen(sockFd, 10);
    if (ret == -1) {
        perror("listen");
        close(sockFd);
        return 1;
    }

    char buf[1024];
    fd_set rdSet;
    fd_set monitorSet;
    int *netFdArr = NULL;
    int curConn = 0;
    int maxConn = 10;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    netFdArr = (int *)malloc(maxConn * sizeof(int));
    if (netFdArr == NULL) {
        perror("malloc");
        close(sockFd);
        return 1;
    }

    FD_ZERO(&monitorSet);
    FD_SET(sockFd, &monitorSet);

    while (1) {
        memcpy(&rdSet, &monitorSet, sizeof(fd_set));
        int maxFd = sockFd;
        for (int i = 0; i < curConn; i++) {
            if (netFdArr[i] > maxFd) {
                maxFd = netFdArr[i];
            }
        }
        ret = select(maxFd + 1, &rdSet, NULL, NULL, NULL);
        if (ret == -1) {
            perror("select");
            break;
        }

        if (FD_ISSET(sockFd, &rdSet)) {
            if (curConn >= maxConn) {
                maxConn *= 2;
                netFdArr = (int *)realloc(netFdArr, maxConn * sizeof(int));
                if (netFdArr == NULL) {
                    perror("realloc");
                    break;
                }
            }
            netFdArr[curConn] = accept(sockFd, (struct sockaddr*)&client_addr, &client_addr_len);
            if (netFdArr[curConn] == -1) {
                perror("accept");
                continue;
            }
            printClient(sockFd, &client_addr, &client_addr_len);
            FD_SET(netFdArr[curConn], &monitorSet);
            curConn++;
        }

        for (int i = 0; i < curConn; i++) {
            if (FD_ISSET(netFdArr[i], &rdSet)) {
                bzero(buf, sizeof(buf));
                ret = recv(netFdArr[i], buf, sizeof(buf), 0);
                if (ret <= 0) {
                    // 处理客户端断开连接或者接收错误
                    handleClientDisconnect(netFdArr, &curConn, i);
                    FD_CLR(netFdArr[i], &monitorSet);
                    continue;
                }

                for (int j = 0; j < curConn; j++) {
                    if (i == j) {
                        continue;
                    }
                    send(netFdArr[j], buf, ret, 0);
                }
            }
        }
    }

    for (int i = 0; i < curConn; i++) {
        close(netFdArr[i]);
    }
    free(netFdArr);
    close(sockFd);
    printf("server close\n");

    return 0;
}