#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 错误处理函数
void handle_error(MYSQL *conn, const char *msg) {
    fprintf(stderr, "%s: %s\n", msg, mysql_error(conn));
    mysql_close(conn);
    exit(1);
}

int main() {
    // 初始化 MySQL 连接句柄
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return 1;
    }

    // 连接到 MySQL 服务器
    if (mysql_real_connect(conn, "localhost", "root", "000000", NULL, 0, NULL, 0) == NULL) {
        handle_error(conn, "mysql_real_connect() failed");
    }

    // 使用数据库
    if (mysql_query(conn, "USE test")) {
        handle_error(conn, "mysql_query() failed while using database");
    }

    // 查询数据
    if (mysql_query(conn, "SELECT * FROM tb_user")) {
        handle_error(conn, "mysql_query() failed while querying data");
    }

    // 获取查询结果集
    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) {
        handle_error(conn, "mysql_store_result() failed");
    }

    // 获取结果集中的列数
    int num_fields = mysql_num_fields(result);
    MYSQL_ROW row; // char **

    // 打印表头
    MYSQL_FIELD *fields = mysql_fetch_fields(result);
    for(int i = 0; i < num_fields; i++) {
        printf("%s \n%s \n", fields[i].name, fields[i].catalog);
    }
    printf("\n");

    // 遍历结果集
    while ((row = mysql_fetch_row(result))) {
        for(int i = 0; i < num_fields; i++) {
            printf("%s ", row[i] ? row[i] : "NULL");
        }
        printf("\n");
    }

    // 释放结果集
    mysql_free_result(result);

    // 关闭 MySQL 连接
    mysql_close(conn);

    return 0;
}