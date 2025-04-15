#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFERSIZE 5
const int TRANSIZE = 20;
int num = 10;
pthread_mutex_t mutex;
pthread_cond_t full, empty;

typedef struct buffer {
   int count;
   int in;
   int out;
   int buf[BUFFERSIZE];
} buffer;

// 正确初始化结构体
buffer buf = { 0, 0, 0, {0} };

void* writer(void* arg) {
   for (int i = 0; i < TRANSIZE; ++i) {
       pthread_mutex_lock(&mutex);
       // 使用 while 循环避免虚假唤醒
       while (buf.count == BUFFERSIZE) {
           pthread_cond_wait(&empty, &mutex);
       }
       buf.buf[buf.in] = rand() % 1000;
       buf.in = (buf.in + 1) % BUFFERSIZE;
       buf.count++;
       // 通知消费者缓冲区有数据了
       pthread_cond_signal(&full);
       pthread_mutex_unlock(&mutex);
   }
   return NULL;
}

void* reader(void* arg) {
   for (int i = 0; i < TRANSIZE; ++i) {
       pthread_mutex_lock(&mutex);
       // 使用 while 循环避免虚假唤醒
       while (buf.count == 0) {
           pthread_cond_wait(&full, &mutex);
       }
       printf("%d\t", buf.buf[buf.out]);
       buf.out = (buf.out + 1) % BUFFERSIZE;
       buf.count--;
       // 通知生产者缓冲区有空位了
       pthread_cond_signal(&empty);
       pthread_mutex_unlock(&mutex);
   }
   return NULL;
}

int main() {
   pthread_t p1;
   pthread_t p2;

   // 初始化互斥锁和条件变量
   pthread_mutex_init(&mutex, NULL);
   pthread_cond_init(&full, NULL);
   pthread_cond_init(&empty, NULL);

   pthread_create(&p1, NULL, writer, NULL);
   pthread_create(&p2, NULL, reader, NULL);

   pthread_join(p1, NULL);
   pthread_join(p2, NULL);

   // 销毁互斥锁和条件变量
   pthread_mutex_destroy(&mutex);
   pthread_cond_destroy(&full);
   pthread_cond_destroy(&empty);

   printf("\nexit 0\n");
   return 0;
}