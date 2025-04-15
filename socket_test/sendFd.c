#include<func231.h>

int sendFd(int pipeFd, int sendFd) {
    struct msghdr hdr;
    bzero(&hdr, sizeof(hdr));

    // 第一部分发送文本数据
    struct iovec iov[1];
    char buf[10] = "12345";
    iov[0].iov_base = buf;
    iov[0].iov_len = 5;
    hdr.msg_iov = iov;
    hdr.msg_iovlen = 1;

    // 第二部分发送控制数据
    struct cmsghdr* pcmsghdr = (struct cmsghdr*)calloc(1, sizeof(struct cmsghdr) + CMSG_LEN(sizeof(int)));
    if (pcmsghdr == NULL) {
        perror("calloc");
        return -1;
    }
    pcmsghdr->cmsg_len = CMSG_LEN(sizeof(int));
    pcmsghdr->cmsg_level = SOL_SOCKET;
    pcmsghdr->cmsg_type = SCM_RIGHTS;
    *(int*)CMSG_DATA(pcmsghdr) = sendFd;
    hdr.msg_control = pcmsghdr;
    hdr.msg_controllen = pcmsghdr->cmsg_len;

    int ret = sendmsg(pipeFd, &hdr, 0);
    ERROR_CHECK(ret, -1, "sendmsg");

    free(pcmsghdr);
    return ret;
}

int recvFd(int pipeFd, int* recvFd) {
    struct msghdr hdr;
    bzero(&hdr, sizeof(hdr));

    // 第一部分发送文本数据
    struct iovec iov[1];
    char buf[10] = {0};
    iov[0].iov_base = buf;
    iov[0].iov_len = sizeof(buf); // 不能为0
    hdr.msg_iov = iov;
    hdr.msg_iovlen = 1;

    // 第二部分发送控制数据
    struct cmsghdr* pcmsghdr = (struct cmsghdr*)calloc(1, CMSG_LEN(sizeof(int)));
    if (pcmsghdr == NULL) {
        perror("calloc");
        return -1;
    }
    pcmsghdr->cmsg_len = CMSG_LEN(sizeof(int));
    pcmsghdr->cmsg_level = SOL_SOCKET;
    pcmsghdr->cmsg_type = SCM_RIGHTS;
    hdr.msg_control = pcmsghdr;
    hdr.msg_controllen = pcmsghdr->cmsg_len;

    int ret = recvmsg(pipeFd, &hdr, 0);
    ERROR_CHECK(ret, -1, "recvmsg");
    *recvFd = *(int*)CMSG_DATA(pcmsghdr);

    free(pcmsghdr);
    return ret;
}


int main(){
    int sv[2];

    socketpair(AF_LOCAL, SOCK_STREAM, 0, sv);

    int pid = fork();
    if(pid == 0){
        close(sv[0]);
        int fd = open("file",  O_RDWR | O_CREAT | O_TRUNC, 0666);
        printf("fd = %d \n", fd);
        write(fd, "12345", 5);
        sendFd(sv[1], fd);
        sleep(10);
        exit(0);
    }
    else if(pid > 0){
        close(sv[1]);
        int fd = open("file1",  O_RDWR | O_CREAT | O_TRUNC, 0666);
        printf("parent fd = %d \n", fd);
        int newFd;
        recvFd(sv[0], &newFd);
        printf("parent newFd = %d \n", newFd);
        wait(NULL);
    }
}