#include <stdio.h>
#include <stdlib.h>
#include "parse.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

void run_shell(Pipe p){
	if(p==NULL)
		return;
	//printf("%s\n",getenv("PWD"));
	Cmd c=p->head;
	if ( !strcmp(c->args[0], "end") )
      exit(0);
 	if(strcmp("cd",c->args[0])==0){//cd has to be done in parent process
		if(c->args[1]!=NULL){
			char res_path[200];
			realpath(c->args[1],res_path);
			printf("%s\n",res_path);
			chdir(res_path);
		}
		else
			chdir(getenv("HOME"));
		return;
	}
	if(strcmp("logout",c->args[0])==0){//exit 
		exit(0);
	}
 	int pid=fork();
	if(pid==0)
	{
		int out,in;
		if(c->out!=Tnil)
			switch(c->out){
				case Tout: out=open(c->outfile,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP);dup2(out,1);break;
				case ToutErr: out=open(c->outfile,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP);dup2(out,1);dup2(out,2);break;
				case Tapp: out=open(c->outfile,O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP);dup2(out,1);break;
				case TappErr: out=open(c->outfile,O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP);dup2(out,1);dup2(out,2);break;
			}
		if(c->in==Tin)
			{in=open(c->infile,O_RDONLY);dup2(in,0);}	
		close(out);
		close(in);
		if(execvp(c->args[0],c->args)<0)
			printf("oh no :(\n");
	}
	else
		wait(NULL);
}
void run_pipeline(Pipe p)
{
	while(1){
		if(p==NULL)
			return;
		run_shell(p);
		p=p->next;
	}
}