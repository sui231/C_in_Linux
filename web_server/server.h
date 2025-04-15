#pragma once
int tcpInit(const char *ip, const char *port);

int epollRun(const char *ip, const char *port);

int acceptConn(int sockFd, int epfd);

int recvHttpRequest(int cfd, int epfd);

int parseRequestLine(int cfd, const char *reqLine);

int sendHeader(int cfd, int status, const char *descr, const char *type, long length);

int sendFile(int cfd, const char *filename);

int sendDir(int cfd, const char *dirName);

int disconnect(int cfd, int epfd);

const char *getFileType(const char *name);

void showTime();

int hexit(char c);

void decodeMsg(char* to, char* from);
