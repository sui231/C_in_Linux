#include "threadPool.h"
#include <func231.h>

int transFile(int netFd){
    int fd = open("file1", O_RDWR);
    ERROR_CHECK(fd,-1,"open");
    //int ret = send(netFd,"file1",5,0);
    //ERROR_CHECK(ret,-1,"send");
    train_t train;
    train.length = 5;
    strcpy(train.buf,"file1");
    int ret = send(netFd,&train,sizeof(train.length)+train.length,MSG_NOSIGNAL);
    struct stat statbuf;
    ret = fstat(fd,&statbuf);
    ERROR_CHECK(ret,-1,"fstat");
    train.length = 4;//车厢是4个字节 int
    int fileSize = statbuf.st_size; // 长度转换成int
    memcpy(train.buf,&fileSize,sizeof(int));//int存入小火车
    send(netFd,&train,train.length+sizeof(train.length),MSG_NOSIGNAL);
    /*
    char *p = (char *)mmap(NULL,fileSize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    ERROR_CHECK(p,MAP_FAILED,"mmap");
    send(netFd,p,fileSize,MSG_NOSIGNAL);
    */
    sendfile(netFd,fd,NULL,fileSize);
    train.length = 0;
    ret = send(netFd,&train,sizeof(train.length)+train.length,MSG_NOSIGNAL);
    close(fd);
}