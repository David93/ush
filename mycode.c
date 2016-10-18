#include <stdio.h>
#include <stdlib.h>
#include "parse.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

void run_shell(Pipe p){
	if(p==NULL)
		return;
	Cmd c=p->head;
	printf("%s\n",getenv("PWD"));
	int pid=fork();
	if(pid==0)
	{
		int out=open(c->outfile,O_WRONLY| O_CREAT);
		if(c->out==Tout)
			dup2(out,1);
		if(execvp(c->args[0],c->args)<0)
			printf("oh no :(\n");
		close(out);
	}
	else
		wait(NULL);

}