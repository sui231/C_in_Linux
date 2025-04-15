#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

int main(void){
	int fd = open("test1.txt", O_RDWR | O_TRUNC | O_CREAT, 0664);
	if(fd == -1){
		perror("open");
		return 1;
	}

	ftruncate(fd, 1024);
	off_t file_size = lseek(fd, 0, SEEK_END);

	
	char* maped = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(maped == MAP_FAILED){
		perror("mmap");
		close(fd);
		return 1;
	}

	strcpy(maped, "abcd");
	printf("%s\n", maped);

	munmap(maped, file_size);
	close(fd);
	return 0;
}
