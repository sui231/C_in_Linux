#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>

int main(int argc, const char* argv[]) {
    // ./chat_client 192.168.152.129 1234
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP address> <port>\n", argv[0]);
        return 1;
    }

    // 创建套接字
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket");
        return 1;
    }

    // 初始化服务器地址结构体
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));

    // 连接服务器
    int ret = connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1) {
        perror("connect");
        close(socket_fd);
        return 1;
    }

    char buf[1024];
    fd_set rfds;

    while (1) {
        // 清空文件描述符集合
        FD_ZERO(&rfds);
        // 添加需要监听的文件描述符
        FD_SET(socket_fd, &rfds);
        FD_SET(STDIN_FILENO, &rfds);

        // 调用 select 函数进行多路复用
        ret = select(socket_fd + 1, &rfds, NULL, NULL, NULL);
        if (ret == -1) {
            perror("select");
            break;
        }

        // 处理标准输入事件
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            bzero(buf, sizeof(buf));
            ret = read(STDIN_FILENO, buf, sizeof(buf));
            if (ret == 0) {
                const char* message = "client close";
                send(socket_fd, message, strlen(message), 0);
                break;
            } else if (ret > 0) {
                // 发送实际读取的字节数
                send(socket_fd, buf, ret, 0);
            }
        }

        // 处理服务器套接字事件
        if (FD_ISSET(socket_fd, &rfds)) {
            bzero(buf, sizeof(buf));
            ret = recv(socket_fd, buf, sizeof(buf), 0);
            if (ret == 0) {
                puts("client recv null");
                break;
            } else if (ret > 0) {
                puts("\n(client recv)");
                puts(buf);
            }
        }
    }

    // 关闭套接字
    close(socket_fd);
    puts("client close");

    return 0;
}