#include <func231.h>
enum {
    FREE,
    BUSY
};
typedef struct workerData_s{
    pid_t pid;
    int status;
    int pipeFd;
} workerData_t;
int makeChild(workerData_t *workerArr, int workerNum);
int eventHanler(int pipeFd);
