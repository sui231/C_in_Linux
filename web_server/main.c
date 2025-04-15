#include "server.h"
#include <func231.h>

// ./main 23111 /home/fjw/code/c/web_server/web-http/
int main(int argc, char* argv[]){
    if(argc != 2 && argc != 3){
        printf("Usage: %s [port] [path] (ip) \n", argv[0]);
        return -1;
    }

    // 切换工作目录
    chdir(argv[2]);

    int epfd = epollRun(NULL, argv[1]);
    
    close(epfd);
    return 0;
}