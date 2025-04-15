#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

void sys_err(const char* str){
	perror(str);
	exit(1);
}

int main(int argc, char* argv[]){
	// > redirection implement and >> appending redirection implement
	if(argc == 4){
		if(!strcmp(argv[3], ">") || !strcmp(argv[3], ">>")){
			int fd2;
			if(!strcmp(argv[3], ">"))
				fd2 = open(argv[3], O_WRONLY|O_CREAT|O_TRUNC, 0644);
			else
				fd2 = open(argv[3], O_WRONLY|O_CREAT|O_TRUNC|O_APPEND, 0644);
			
			if(fd2 == -1)
				sys_err("fd2 open(>/>>)");


			dup2(fd2, STDOUT_FILENO); 
			// cover STDOUT_FILENO, write to STDOUT_FILENO below actually write to fd2
			close(fd2);
		}
	}

	if(argc != 2){
		printf("Usage:  imp_cat filename    OR\nimp_cat filename1 > filename2    OR\nimp_cat filename1 >> filename2\n");
		exit(0);
	}

	int fd1 = open(argv[1], O_RDONLY);
	if(fd1 == -1)
		sys_err("fd1 open");	
	int n;
	char buf[1024];
	while(n = read(fd1, buf, sizeof(buf))){
		write(STDOUT_FILENO, buf, n);
	}
	close(fd1);
	
	return 0;
}
