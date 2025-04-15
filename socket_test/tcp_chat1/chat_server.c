#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

int main(int argc, const char* argv[]) {
    // ./chat_server 192.168.152.129 1234
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP address> <port>\n", argv[0]);
        return 1;
    }

    // 创建套接字
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        return 1;
    }

    // 设置 SO_REUSEADDR 选项
    int opt = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(sock_fd);
        return 1;
    }

    // 初始化服务器地址结构体
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));

    // 绑定地址和端口
    int ret = bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1) {
        perror("bind");
        close(sock_fd);
        return 1;
    }

    // 开始监听连接请求
    ret = listen(sock_fd, 10);
    if (ret == -1) {
        perror("listen");
        close(sock_fd);
        return 1;
    }

    // 接受客户端连接
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int net_fd = accept(sock_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    //int net_fd = accept(sock_fd, NULL, NULL);
    if (net_fd == -1) {
        perror("accept");
        close(sock_fd);
        return 1;
    }

    // 打印客户端信息
    printf("net_fd = %d\n", net_fd);
    printf("client_addr_len = %d\n", client_addr_len);
    printf("client family = %d\n", client_addr.sin_family);
    printf("client port = %d\n", ntohs(client_addr.sin_port));
    printf("client address = %s\n", inet_ntoa(client_addr.sin_addr));

    char buf[1024];
    fd_set rfds;

    while (1) {
        // 清空文件描述符集合
        FD_ZERO(&rfds);
        // 添加需要监听的文件描述符
        FD_SET(net_fd, &rfds);
        FD_SET(STDIN_FILENO, &rfds);

        // 调用 select 函数进行多路复用
        ret = select(net_fd + 1, &rfds, NULL, NULL, NULL);
        if (ret == -1) {
            perror("select");
            break;
        }

        // 处理标准输入事件
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            bzero(buf, sizeof(buf));
            ret = read(STDIN_FILENO, buf, sizeof(buf));
            if (ret == 0) {
                const char* message = "server close";
                send(net_fd, message, strlen(message), 0);
                break;
            } else if (ret > 0) {
                // 发送实际读取的字节数
                send(net_fd, buf, ret, 0);
            }
        }

        // 处理客户端套接字事件
        if (FD_ISSET(net_fd, &rfds)) {
            bzero(buf, sizeof(buf));
            ret = recv(net_fd, buf, sizeof(buf), 0);
            if (ret == 0) {
                puts("server recv null");
                break;
            } else if (ret > 0) {
                puts("server recv:");
                puts(buf);
            }
        }
    }

    // 关闭套接字
    close(net_fd);
    close(sock_fd);
    printf("server close\n");

    return 0;
}