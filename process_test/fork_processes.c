#include <stdio.h>
#include <unistd.h>

int main() {
	int i;
	for(i = 0; i < 5; ++i){
        if(!fork())
            break;
    }
    if(i == 5){
        sleep(5);
        printf("parent process\n");
    } else {
        sleep(i);
        printf("child process%d\n", i);
    }
    printf("Process PID: %d\n", getpid());
    return 0;
}
