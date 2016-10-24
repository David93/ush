#include <stdio.h>
#include <stdlib.h>
#include "parse.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <string.h>
#include <signal.h>

extern char **environ;
void redirect(Cmd c){
	int out,in;
	if(c->out!=Tnil){
		switch(c->out){
			case Tout: out=open(c->outfile,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP);dup2(out,1);break;
			case ToutErr: out=open(c->outfile,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP);dup2(out,1);dup2(out,2);break;
			case Tapp: out=open(c->outfile,O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP);dup2(out,1);break;
			case TappErr: out=open(c->outfile,O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP);dup2(out,1);dup2(out,2);break;
		}
		close(out);
	}
	if(c->in==Tin)
		{in=open(c->infile,O_RDONLY);dup2(in,0);close(in);}
}
void signal_handle(int parent){
	sigset_t mask;
  	sigset_t orig_mask;
    sigemptyset (&mask);
    sigaddset (&mask, SIGQUIT);
    sigaddset (&mask, SIGTERM);
    if(parent==1)
    	sigaddset(&mask, SIGINT);
    else
    {
    	sigset_t mask2;
  		sigset_t orig_mask2;
    	sigemptyset (&mask2);
    	sigaddset(&mask2, SIGINT);
    	if (sigprocmask(SIG_UNBLOCK, &mask2, &orig_mask2) < 0) {
	    	printf("signal handle :(\n");
	    	return;
  		}
	}
    if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0) {
        printf("signal handle :(\n");   }
}
void run_cmd(Cmd c){
	//if(c==NULL)
	//	return;
	//printf("%s\n",getenv("PWD"));
	//Cmd c=p->head;
	if ( !strcmp(c->args[0], "end") )
    {//printf("Exiting on end\n"); 
    return;}
 	if(strcmp("cd",c->args[0])==0){//cd has to be done in parent process
		int x;
		if(c->args[1]!=NULL){
			char res_path[200];
			realpath(c->args[1],res_path);
			//printf("%s\n",res_path);
			x=chdir(res_path);
		}
		else
			x=chdir(getenv("HOME"));
		if(x<0)printf("%d cd :(\n",x);
		return;
	}
	if(strcmp("logout",c->args[0])==0){//exit 
		exit(0);
	}
	if(strcmp("setenv",c->args[0])==0){//setenv handler
		if(c->args[1]==NULL){
			int pid=fork();
			if(pid==0){
			redirect(c);
			int i = 1;
  			char *s = *environ;
			for (; s; i++) {
    		printf("%s\n", s);
   			 s = *(environ+i);
  			}
  			exit(0);
  			}
  			else
  				wait(NULL);
  			return;
  			
		}
		int x=setenv(c->args[1],c->args[2],1);
		if(x<0)printf("%d setenv :(\n",x);
		//printf("%s\n",getenv(c->args[1]));
		return;
	}
	if(strcmp("unsetenv",c->args[0])==0){//unsetenv handler
		int x=unsetenv(c->args[1]);
		if(x<0)printf("%d unsetenv :(\n",x);
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
			redirect(c);
			if(execvp(c->args[2],c->args+2)<0)
				printf("oh no :(\n");
		}
		else
			wait(NULL);
		return;
	}
	if(strcmp("where",c->args[0])==0){
		int pid=fork();
		if(pid==0)
		{
			int i=1;
			char **newargs = malloc((c->nargs+1)*sizeof(char*));
			newargs[0]=c->args[0];
			char *k="-b";
			newargs[1]=k;
			int j;

			for(j=2;j<c->nargs+2;j++)
				newargs[j]=c->args[j-1];
			redirect(c);
			if(execvp("whereis",newargs)<0)
				printf("oh no :(\n");
		}
		else
			wait(NULL);
		return;	
	}
	
	int pid=fork();
	if(pid==0)
	{
		redirect(c);
		signal_handle(0);
		if(execvp(c->args[0],c->args)<0)
			printf("oh no :(\n");
	}
	else
		wait(NULL);
}
void create_proc(int in, int out, Cmd c){
	int pid=fork();
	if(pid==0){
		if(c->in==Tin)
			 { int k=open(c->infile,O_RDONLY);dup2(k,0);close(k);}
		if (in != 0)
        {
          	dup2 (in, 0);
          close (in);
        }
        
        
      if (out != 1)
        {
        	if(c->out==TpipeErr)
			 { dup2(out,2);}
          dup2 (out, 1);
          close (out);
        }
        if(strcmp("where",c->args[0])==0){
			int i=1;
			char **newargs = malloc((c->nargs+1)*sizeof(char*));
			newargs[0]=c->args[0];
			char *k="-b";
			newargs[1]=k;
			int j;

			for(j=2;j<c->nargs+2;j++)
				newargs[j]=c->args[j-1];
			if(execvp("whereis",newargs)<0)
				printf("oh no :(\n");
		}
		if(strcmp("nice",c->args[0])==0){//nice handler
		int val=atoi(c->args[1]);
			if(c->args[1]=='0')
				setpriority(PRIO_PROCESS,getpid(),0);
			else
			{
				if(val==0)
					setpriority(PRIO_PROCESS,getpid(),4);
				else
					setpriority(PRIO_PROCESS,getpid(),val);
			}
			if(execvp(c->args[2],c->args+2)<0)
				printf("oh no :(\n");
		}
        if(execvp(c->args[0],c->args)<0)
		printf("oh no :(\n");

	}
	else
		wait(NULL);
}
void run_cmd_pipes(Cmd c){
	int in,fd[2];
	in=0;

	while(c->next!=NULL){
		pipe(fd);
		create_proc(in,fd[1],c);
		close(fd[1]);
		in=fd[0];
		c=c->next;
	}
	int pid=fork();
	if(pid==0){
		if(in!=0)
			dup2(in,0);
		int out;
		if(c->out!=Tnil)
		switch(c->out){
			case Tout: out=open(c->outfile,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP);dup2(out,1);break;
			case ToutErr: out=open(c->outfile,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP);dup2(out,1);dup2(out,2);break;
			case Tapp: out=open(c->outfile,O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP);dup2(out,1);break;
			case TappErr: out=open(c->outfile,O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP);dup2(out,1);dup2(out,2);break;
		}
		if(execvp(c->args[0],c->args)<0)
			printf("oh no :(\n");
	}
	else 
		wait(NULL);
}
void run_pipe(Pipe p){
	Cmd c=p->head;
	if(c->next==NULL)
	{	run_cmd(c); return;}
	run_cmd_pipes(c);
	/*
	while(1){
		run_cmd(c);
		c=c->next;
		if(c==NULL)
			return;
	}
	*/
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