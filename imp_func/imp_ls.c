#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>

void sys_err(const char* str){
	perror(str);
	exit(1);
}

int main(int argc, char* argv[]){
	if(argc != 2){
		printf("Usage: imp_ls <dir>\n");
		exit(0);
	}

	DIR* dp;
	struct dirent* sdp;
	
	dp = opendir(argv[1]);
	if(dp == NULL){
		sys_err("opendir error\n");
	}
	
	int i = 0;
	while((sdp = readdir(dp)) != NULL){
		if(sdp->d_name[0] != '.')
			printf("%-10s %c", sdp->d_name, ++i % 5 ? '\t' : '\n');
	}
	if(i % 5)
		printf("\n");

	closedir(dp);
	return 0;
}
