#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void myfunc(int sig)
{
    printf("signal %d, wait 3s\n", sig);
    sleep(3);
    printf("wake up .....\n");
}

int main()
{
    // 注册信号捕捉函数
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = myfunc;

    // 设置临时屏蔽的信号
    sigemptyset(&act.sa_mask);  // 清空
    sigaddset(&act.sa_mask, SIGQUIT);

    // 注册信号并检查返回值
    if (sigaction(SIGINT, &act, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    while (1) {
        sleep(1);
    }

    return 0;
}
