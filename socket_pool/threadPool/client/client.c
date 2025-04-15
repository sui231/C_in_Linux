#include <43func.h>
typedef struct train_s{
    int length;
    char buf[1000];
} train_t;
int recvn(int sockFd,void *pstart,int len){
    int total = 0;
    int ret;
    char *p = (char *)pstart;
    while(total < len){
        ret = recv(sockFd,p+total,len-total,0);
        total += ret;
    }
    return 0;
}
int recvFile(int sockFd){
    //先获取文件名
    char name[1024] = {0};
    int dataLength;
    //int ret = recv(sockFd,&dataLength,sizeof(int),MSG_WAITALL);
    int ret = recvn(sockFd,&dataLength,sizeof(int));
    ERROR_CHECK(ret,-1,"recv");
    //ret = recv(sockFd,name,dataLength,MSG_WAITALL);
    ret = recvn(sockFd,name,dataLength);
    ERROR_CHECK(ret,-1,"recv");
    int fd = open(name,O_RDWR|O_CREAT|O_TRUNC,0666);
    ERROR_CHECK(fd,-1,"open");
    int fileSize;
    recvn(sockFd,&dataLength,sizeof(int));
    recvn(sockFd,&fileSize,dataLength);
    printf("fileSize = %d\n", fileSize);
    char buf[1000] = {0};
    time_t timeBeg,timeEnd;
    timeBeg = time(NULL);

    int pipefds[2];
    pipe(pipefds);
    int total = 0;
    while(total < fileSize){
        int ret = splice(sockFd,NULL,pipefds[1],NULL,4096,SPLICE_F_MORE);
        total += ret;
        //usleep(2000000);
        splice(pipefds[0],NULL,fd,NULL,ret,SPLICE_F_MORE);
    }
    /*
    ftruncate(fd,fileSize);
    char *p = (char *)mmap(NULL,fileSize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    ERROR_CHECK(p,MAP_FAILED,"mmap");
    recvn(sockFd,p,fileSize);
    */
    recvn(sockFd,&dataLength,sizeof(int));
    printf("dataLength = %d\n", dataLength);
    timeEnd = time(NULL);
    printf("total time = %ld\n", timeEnd - timeBeg);
}
int main(int argc, char *argv[]){
    // ./client 192.168.14.9 1234
    ARGS_CHECK(argc,3);
    int sockFd = socket(AF_INET,SOCK_STREAM,0);
    ERROR_CHECK(sockFd,-1,"socket");
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    int ret = connect(sockFd,(struct sockaddr *)&addr,sizeof(addr));
    ERROR_CHECK(ret,-1,"connect");
    recvFile(sockFd);
    close(sockFd);
}