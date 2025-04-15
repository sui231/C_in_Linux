#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

const int BUFFSIZE = 1024;

int main(int argc, char* argv[]) {
    if (argc != 3) {  // 检查命令行参数个数
        fprintf(stderr, "Usage: %s <source file> <destination file>\n", argv[0]);
        exit(1);
    }

    const char* PATH = argv[1];  // 第一个命令行参数：源文件
    //const char* DEST_PATH = argv[2];  // 第二个命令行参数：目标文件
    const char* NEW_PATH = argv[2];  // 第二个命令行参数：目标文件
	char DEST_PATH[20];
	snprintf(DEST_PATH, sizeof(DEST_PATH), "new_%s", NEW_PATH);

    int src = open(PATH, O_RDONLY);  // 打开源文件
    int new = open(DEST_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0664);  // 打开目标文件

    if (src < 0 || new < 0) {
        perror("open error");
        exit(1);
    }

    int num1 = 0;
    char buf[BUFFSIZE];  // 缓冲区

    // 读取源文件并写入目标文件
    while ((num1 = read(src, buf, BUFFSIZE)) > 0) {
        if (num1 < 0) {
            perror("read error");
            exit(1);
        }

        if (write(new, buf, num1) < 0) {  // 写入读取的内容
            perror("write error");
            exit(1);
        }
    }

    if (num1 < 0) {  // 如果读取过程中出现错误
        perror("read error");
        exit(1);
    }

    // 关闭文件
    close(src);
    close(new);

    printf("File copied to: %s\n", DEST_PATH);  // 输出目标文件路径

    return 0;
}

