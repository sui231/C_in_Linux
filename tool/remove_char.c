#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 移除字符串中指定的字符
void remove_char(char *str, char c) {
    int i, j = 0;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] != c) {
            str[j] = str[i];
            j++;
        }
    }
    str[j] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <character> <filename>\n", argv[0]);
        return 1;
    }

    if (strlen(argv[1]) != 1) {
        fprintf(stderr, "Error: The character to remove must be a single character.\n");
        return 1;
    }

    char c = argv[1][0];
    FILE *file = fopen(argv[2], "r");
    if (file == NULL) {
        perror("Failed to open file for reading");
        return 1;
    }

    // 读取文件内容到动态分配的缓冲区
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return 1;
    }

    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';
    fclose(file);

    // 移除指定字符
    remove_char(buffer, c);

    // 以写入模式重新打开文件
    file = fopen(argv[2], "w");
    if (file == NULL) {
        perror("Failed to open file for writing");
        free(buffer);
        return 1;
    }

    // 将修改后的内容写回文件
    fwrite(buffer, 1, strlen(buffer), file);
    fclose(file);
    free(buffer);

    printf("File updated successfully.\n");
    return 0;
}
