#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main()
{
    // 自定义信号集
    sigset_t myset, old;
    sigemptyset(&myset); // 清空

    // 添加要阻塞的信号
    sigaddset(&myset, SIGINT);
    sigaddset(&myset, SIGQUIT);
    // 移除对 SIGKILL 的添加，因为 SIGKILL 不能被阻塞
    // sigaddset(&myset, SIGKILL);

    // 自定义信号集设置到内核中的阻塞信号集
    sigprocmask(SIG_BLOCK, &myset, &old);

    sigset_t pend;
    int i = 0;
    while (1)
    {
        // 读内核中的未决信号集的状态
        sigpending(&pend);
        printf("%d second:\n", i);
        for (int j = 1; j < 32; ++j)
        {
            if (sigismember(&pend, j))
            {
                printf("1 ");
            }
            else
            {
                printf("0 ");
            }
        }
        printf("\n");
        sleep(1);
        i++;

        // 10s之后解除阻塞
        if (i > 5)
        {
            // sigprocmask(SIG_UNBLOCK, &myset, NULL);
            sigprocmask(SIG_SETMASK, &old, NULL);
        }
    }

    return 0;
}
