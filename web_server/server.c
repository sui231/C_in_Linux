#include <func231.h>
#include "server.h"

int epollRun(const char* ip, const char* port){
    int sockFd = tcpInit(ip, port);

    int epfd = epoll_create(1);
    ERROR_CHECK(epfd, -1, "epoll_create");

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = sockFd;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sockFd, &event);
    ERROR_CHECK(ret, -1, "epoll_ctl");


    struct epoll_event eventList[1024];
    int size = sizeof(eventList) / sizeof(eventList[0]);
    while(1){
        printf("\nepoll wait...\n");
        int num = epoll_wait(epfd, (struct epoll_event *)&eventList, size, -1);
        ERROR_CHECK(num, -1, "epoll_wait");

        for(int i = 0; i < num; i++){
            int curFd = eventList[i].data.fd;
            // 有新的连接
            if(curFd == sockFd){
                printf("new connect accept\n");
                acceptConn(sockFd, epfd);
            }
            // 通信
            else{
                printf("receive request\n");
                recvHttpRequest(curFd, epfd);
            }
        }

    }
    return epfd;
}

int tcpInit(const char* ip, const char* port){
    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(sockFd, -1, "socket");

    int optval = 1;
    int ret = setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    ERROR_CHECK(ret, -1, "setsockopt");

    struct sockaddr_in addr;
    if(ip == NULL){
        //如果这样使用 0.0.0.0,任意ip将可以连接
        addr.sin_addr.s_addr = INADDR_ANY;
    }else{
        ret = inet_pton(AF_INET, ip, &addr.sin_addr.s_addr);
        ERROR_CHECK(ret ,-1 , "inet_pton");
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(port));
    ret = bind(sockFd, (struct sockaddr*)&addr, sizeof(addr));
    ERROR_CHECK(ret, -1, "bind");

    ret = listen(sockFd, 100);
    ERROR_CHECK(ret, -1, "listen");

    return sockFd;
}

int acceptConn(int sockFd, int epfd){
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    int newFd = accept(sockFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
    ERROR_CHECK(newFd, -1, "accept");

    // 将新连接的套接字设置为非阻塞模式
    int flags = fcntl(newFd, F_GETFL, 0);
    ERROR_CHECK(flags, -1, "fcntl");
    int ret = fcntl(newFd, F_SETFL, flags | O_NONBLOCK);
    ERROR_CHECK(ret, -1, "fcntl F_SETFL O_NONBLOCK");

    // 打印另一端信息
    char clientIP[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
    printf("Client connected: IP = %s, Port = %d\n"
        , clientIP, ntohs(clientAddr.sin_port));
    ERROR_CHECK(newFd, -1, "accept");

    showTime();

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET; // 边缘触发
    event.data.fd = newFd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, newFd, &event);
    ERROR_CHECK(ret, -1, "epoll_ctl");

    return newFd;
}

int recvHttpRequest(int cfd, int epfd){
    char temp[1024] = {0};
    char buf[4096] = {0};

    // total 接受总数
    int len, total = 0;
    while ((len = recv(cfd, temp, sizeof(temp), 0)) > 0) {
        // 空间充足
        if (total + len < sizeof(buf)) {
            memcpy(buf + total, temp, len);
        }
        total += len;
        printf("recvHttpRequest while loop(%d B request)\n", total);
    }

    // 取出请求行
    if(len == -1 && errno == EAGAIN){
        printf("parseRequestLine\n");
        char* p = strstr(buf, "\r\n");
        if (p != NULL) {
            int reqlen = p - buf;
            buf[reqlen] = '\0';
            parseRequestLine(cfd, buf);
        }
        return -1;
    }
    else if(len == 0){
        printf("client close\n");
        disconnect(cfd, epfd);
        return 0;
    }
    else{
        perror("recv error");
        close(cfd);
        return -1;
    }
    /*
    while (1) {
        int len = recv(cfd, temp, sizeof(temp), 0);
        if (len == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 无数据可读，退出循环，等待下次 epoll 事件
                break;
            } else {
                perror("recv");
                close(cfd);
                return -1;
            }
        } else if (len == 0) {
            // 客户端关闭连接
            printf("client close\n");
            disconnect(cfd, epfd);
            return 0;
        } else {
            // 处理接收的数据
            if (total + len > sizeof(buf)) {
                // 缓冲区不足，截断数据并记录错误（或扩展缓冲区）
                int remaining = sizeof(buf) - total;
                memcpy(buf + total, temp, remaining);
                total = sizeof(buf);
                printf("Warning: Request truncated\n");
                break;
            }
            memcpy(buf + total, temp, len);
            total += len;
        }
    }
    */
}

int disconnect(int cfd, int epfd){
    int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
    ERROR_CHECK(ret, -1, "epoll_ctl");

    close(cfd);
    return 0;
}

// GET /index.html HTTP/1.1
int parseRequestLine(int cfd, const char* reqLine){
    char method[16] = {0};
    char path[1024] = {0};
    char protocol[16] = {0};

    sscanf(reqLine, "%[^ ] %[^ ] %[^ ]", method, path, protocol);

    // 相对路径获取
    char* file = NULL;

    // 还原 Unicode
    decodeMsg(path, path);

    if(strcmp(path, "/") == 0){
        file = "./";
    }
    else{
        file = path + 1;
    }
    printf("file/dir name: %s\n", file);


    if(strcmp(method, "GET") == 0){
        printf("GET\n");
    }
    // else if(strcmp(method, "POST") == 0){
    //     printf("POST\n");
    // }
    else{
        printf("not GET\n");
        return -1;
    }

    // 获取文件类型
    struct stat st;
    int ret = stat(file, &st);
    if(ret == -1){
        // 文件不存在，发送404页面
        perror("stat");
        printf("File path: %s\n", file);
        sendHeader(cfd, 404, "Not Found", getFileType(".jpg"), -1);
        sendFile(cfd, "404.jpg");
    }

    if(S_ISDIR(st.st_mode)){
        // 目录
        printf("send dir\n");
        sendHeader(cfd, 200, "OK", ".html", -1);
        sendDir(cfd, file);
    }
    else {
        // 文件
        printf("send file\n");
        sendHeader(cfd, 200, "OK", getFileType(file), (long)st.st_size);
        sendFile(cfd, file);
    }

    return 0;
}

int sendHeader(int cfd, int status, const char* descr, const char* type, long length){
    // 消息行 + 消息报头 + 空行
    char buf[1024] = {0};
    sprintf(buf, "HTTP/1.1 %d %s\r\n", status, descr);
    sprintf(buf + strlen(buf), "Content-Type: %s\r\n", type);
    sprintf(buf + strlen(buf), "Content-Length: %ld\r\n", length);
    sprintf(buf + strlen(buf), "\r\n");
    send(cfd, buf, strlen(buf), 0);
}

int sendFile(int cfd, const char* fileName){
    int fd = open(fileName, O_RDONLY);
    if (fd == -1) {
        perror("open");
        printf("File path: %s\n", fileName);
        return -1;
    }

    char buf[4096] = {0};
    int len;
    while((len = read(fd, buf, sizeof(buf))) > 0){
        send(cfd, buf, len, 0);
        // 防止发送太快，接收端解析不全
        usleep(50);
    }
    if (len == -1) {
        perror("read");
    }
    close(fd);
    return 0;
}

int sendDir(int cfd, const char* dirName){
    struct dirent** nameList;
    char buf[4096];
    int ret;

    sprintf(buf, "<html><head><title>%s</title></head><body><table>", dirName);

    int num = scandir(dirName, &nameList, NULL, alphasort);
    for(int i = 0; i < num; i++){
        char* name = nameList[i]->d_name;
        char subpath[1024];
        sprintf(subpath, "%s/%s", dirName, name);

        struct stat st;
        ret = stat(subpath, &st);
        ERROR_CHECK(ret, -1, "stat");

        if(S_ISDIR(st.st_mode)){
            // 加上了 /
            sprintf(buf + strlen(buf), "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>", name, name, (long)st.st_size);
        }
        else{
            sprintf(buf + strlen(buf), "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>", name, name, (long)st.st_size);
        }
        ret = send(cfd, buf, strlen(buf), 0);
        ERROR_CHECK(ret, -1, "send");

        memset(buf, 0, sizeof(buf));
        free(nameList[i]);
    }
    sprintf(buf, "</table></head></html>");
    ret = send(cfd, buf, strlen(buf), 0);
    ERROR_CHECK(ret, -1, "send");

    if(nameList){
        free(nameList);
    }

    return 0;
}

const char* getFileType(const char* name){
    const char* dot = strrchr(name, '.');
    if(dot == NULL){
        return "text/plain";
    }

    if(strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0){
        return "text/html";
    }
    else if(strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0){
        return "image/jpeg";
    }
    else if(strcmp(dot, ".gif") == 0){
        return "image/gif";
    }
    else if(strcmp(dot, ".png") == 0){
        return "image/png";
    }
    else if(strcmp(dot, ".css") == 0){
        return "text/css";
    }
    else if(strcmp(dot, ".au") == 0){
        return "audio/basic";
    }
    else if(strcmp(dot, ".wav") == 0){
        return "audio/wav";
    }
    else if(strcmp(dot, ".avi") == 0){
        return "video/x-msvideo";
    }
    else if(strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0){
        return "video/quicktime";
    }
    else if(strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0){
        return "video/mpeg";
    }
    else if(strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0){
        return "model/vrml";
    }
    else if(strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0){
        return "audio/midi";
    }
    else if(strcmp(dot, ".mp3") == 0){
        return "audio/mpeg";
    }
    else if(strcmp(dot, ".ogg") == 0){
        return "application/ogg";
    }
    else if(strcmp(dot, ".pac") == 0){
        return "application/x-ns-proxy-autoconfig";
    }

    return "text/plain";
}

void showTime(){
    time_t current_time;
    struct tm *local_time;

    // 获取当前时间
    time(&current_time);

    // 将当前时间转换为本地时间
    local_time = localtime(&current_time);

    // 定义时间格式并打印
    char time_str[80];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
    printf("current time: %s\n", time_str);
}

int hexit(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return 0;
}

// 对字符串进行解码
void decodeMsg(char* to, char* from) {
    for (; *from != '\0'; ++to, ++from) {
        if (from[0] == '%' && isxdigit((unsigned char)from[1]) && isxdigit((unsigned char)from[2])) {
            // 解码十六进制转义序列
            *to = (hexit(from[1]) << 4) | hexit(from[2]);
            from += 2;
        } else {
            // 直接复制字符
            *to = *from;
        }
    }
    *to = '\0'; // 字符串结束符
}