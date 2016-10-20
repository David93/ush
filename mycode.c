#include <stdio.h>
#include <stdlib.h>
#include "parse.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <string.h>

#define READ_END 0
#define WRITE_END 1
extern char **environ;
void run_cmd(Cmd c){
	//if(c==NULL)
	//	return;
	//printf("%s\n",getenv("PWD"));
	//Cmd c=p->head;
	if ( !strcmp(c->args[0], "end") )
    {//printf("Exiting on end\n"); 
    return;}
 	if(strcmp("cd",c->args[0])==0){//cd has to be done in parent process
		if(c->args[1]!=NULL){
			char res_path[200];
			realpath(c->args[1],res_path);
			//printf("%s\n",res_path);
			chdir(res_path);
		}
		else
			chdir(getenv("HOME"));
		return;
	}
	if(strcmp("logout",c->args[0])==0){//exit 
		exit(0);
	}
	if(strcmp("setenv",c->args[0])==0){//setenv handler
		if(c->args[1]==NULL){
			int i = 1;
  			char *s = *environ;
			for (; s; i++) {
    		printf("%s\n", s);
   			 s = *(environ+i);
  			}
  			return;
		}
		int x=setenv(c->args[1],c->args[2],1);
		if(x<0)printf("%d setenv :(",x);
		printf("%s\n",getenv(c->args[1]));
		return;
	}
	if(strcmp("unsetenv",c->args[0])==0){//unsetenv handler
		int x=unsetenv(c->args[1]);
		if(x<0)printf("%d unsetenv :(",x);
		//printf("%s\n",getenv(c->args[1]));
		return;
	}
	if(strcmp("nice",c->args[0])==0){//nice handler
		int val=atoi(c->args[1]);
		int pid=fork();
		if(pid==0)
		{
			if(c->args[1]=='0')
				setpriority(PRIO_PROCESS,getpid(),0);
			else
			{
				if(val==0)
					setpriority(PRIO_PROCESS,getpid(),4);
				else
					setpriority(PRIO_PROCESS,getpid(),val);
			}
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
			if(execvp(c->args[2],c->args+2)<0)
				printf("oh no :(\n");
		}
		else
			wait(NULL);
			return;
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
void run_pipe(Pipe p){
	Cmd c=p->head;
	while(1){
		run_cmd(c);
		c=c->next;
		if(c==NULL)
			return;
	}
}
void run_shell(Pipe p)
{
	while(1){
		run_pipe(p);
		p=p->next;
		if(p==NULL)
			return;
	}
}