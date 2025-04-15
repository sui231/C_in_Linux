#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/stat.h>

int main(int argc,char* argv[]){
	struct stat sbuffer;

	int ret = stat(argv[1], &sbuffer);
	if(ret == -1){
		perror("stat error");
		exit(1);
	}

	printf("size: %ld\n", sbuffer.st_size);

	if(S_ISREG(sbuffer.st_mode)){
		printf("regular file\n");
	} else if(S_ISDIR(sbuffer.st_mode)){
		printf("directoary\n");
	}else if(S_ISFIFO(sbuffer.st_mode)){
		printf("fifo\n");
	}
	return 0;
}
