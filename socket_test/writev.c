#include<func231.h>
int main(){
    int fds[2];
    pipe(fds);

    if(fork() == 0){
        close(fds[1]);
        char buf[10] = {0};
        struct iovec iov[1];
        iov[0].iov_base = buf;
        iov[0].iov_len = sizeof(buf);

        readv(fds[0], iov, 1);
        printf("I am child, buf = %s\n", buf);
        exit(0);
    }
    else{
        close(fds[0]);
        char buf[10] = "hello";
        struct iovec iov[1];
        iov[0].iov_base = buf;
        iov[0].iov_len = strlen(buf);

        writev(fds[1], iov, 1);
        wait(NULL);
    }
}