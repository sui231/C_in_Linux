#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

int main(int argc, char *argv[]){
    // ./client_chat 192.168.152.129 1234
    ARGS_CHECK(argc,3);
    int sockFd = socket(AF_INET,SOCK_DGRAM,0);
    ERROR_CHECK(sockFd,-1,"socket");
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    char buf[1024] = "I am comming";
    int ret = sendto(sockFd,buf,strlen(buf),0,(struct sockaddr *)&addr, sizeof(addr));
    ERROR_CHECK(ret,-1,"recvfrom");
    fd_set rdset;
    while(1){
        FD_ZERO(&rdset);
        FD_SET(STDIN_FILENO,&rdset);
        FD_SET(sockFd,&rdset);
        select(sockFd+1,&rdset,NULL,NULL,NULL);
        puts("select returns");
        if(FD_ISSET(STDIN_FILENO,&rdset)){
            bzero(buf,sizeof(buf));
            int ret = read(STDIN_FILENO,buf,sizeof(buf));
            if(ret == 0){
                sendto(sockFd,buf,0,0,(struct sockaddr *)&addr,sizeof(addr));
                break;
            }
            sendto(sockFd,buf,strlen(buf),0,(struct sockaddr *)&addr,sizeof(addr));
        }
        if(FD_ISSET(sockFd,&rdset)){
            bzero(buf,sizeof(buf));
            socklen_t addrLen = sizeof(addr);
            int ret = recvfrom(sockFd,buf,sizeof(buf),0,(struct sockaddr *)&addr,&addrLen);
            if(ret == 0){
                break;
            }
            puts(buf);
        }
    } 
    close(sockFd);
}