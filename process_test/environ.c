#include <stdio.h>

extern char** environ;

int main(){
	for(int i = 0; environ[i] != NULL ; ++i){
		printf("%s\ni = %d\n\n", environ[i], i);
	}

	return 0;
}
