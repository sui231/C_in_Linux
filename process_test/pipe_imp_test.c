#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>

void sys_err(const char* str){
	perror(str);
	exit(1);
}

int main(int argc, char* argv[]){
	int fd_pipe[2] = { 0 };
	if(pipe(fd_pipe)){
		sys_err("pipe");
	}

	pid_t pid = fork();
	
	if(pid > 0){
		close(fd_pipe[0]);
		execlp("ls", "ls", NULL);
		dup2(fd_pipe[1], STDOUT_FILENO);
		if(waitpid(pid, NULL, 0) == -1){
			sys_err("wait");
		}
	} else if(pid < 0){
		close(fd_pipe[1]);
		execlp("wc", "wc", "-l", NULL);
		dup2(fd_pipe[0], STDIN_FILENO);
	} else{
		sys_err("fork");
	}

	return 0;
}
