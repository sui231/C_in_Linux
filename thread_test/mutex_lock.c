#include <pthread.h>
#include <stdio.h>

int num = 10;
pthread_mutex_t mutex;

void* thread1(void* arg) {
	pthread_mutex_lock(&mutex);
	--num;
	printf("%d\n", num);
	pthread_mutex_unlock(&mutex);
}

void* thread2(void* arg) {
	pthread_mutex_lock(&mutex);
	++num;
	printf("%d\n", num);
	pthread_mutex_unlock(&mutex);
}

int main() {
	pthread_t p1;
	pthread_t p2;

	pthread_create(&p1, NULL, thread1, NULL);
	pthread_create(&p2, NULL, thread2, NULL);

	pthread_join(p1, NULL);
	pthread_join(p2, NULL);

	pthread_mutex_destroy(&mutex);

	return 0;
}