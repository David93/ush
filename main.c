/******************************************************************************
 *
 *  File Name........: main.c
 *
 *  Description......: Simple driver program for ush's parser
 *
 *  Author...........: Vincent W. Freeh
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "parse.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <string.h>

static void prCmd(Cmd c)
{
  int i;

  if ( c ) {
    printf("%s%s ", c->exec == Tamp ? "BG " : "", c->args[0]);
    if ( c->in == Tin )
      printf("<(%s) ", c->infile);
    if ( c->out != Tnil )
      switch ( c->out ) {
      case Tout:
	printf(">(%s) ", c->outfile);
	break;
      case Tapp:
	printf(">>(%s) ", c->outfile);
	break;
      case ToutErr:
	printf(">&(%s) ", c->outfile);
	break;
      case TappErr:
	printf(">>&(%s) ", c->outfile);
	break;
      case Tpipe:
	printf("| ");
	break;
      case TpipeErr:
	printf("|& ");
	break;
      default:
	fprintf(stderr, "Shouldn't get here\n");
	exit(-1);
      }

    if ( c->nargs > 1 ) {
      printf("[");
      for ( i = 1; c->args[i] != NULL; i++ )
	printf("%d:%s,", i, c->args[i]);
      printf("\b]");
    }
    putchar('\n');
    // this driver understands one command
    if ( !strcmp(c->args[0], "end") )
      exit(0);
      
  }
}

static void prPipe(Pipe p)
{
  int i = 0;
  Cmd c;

  if ( p == NULL )
    return;

  printf("Begin pipe%s\n", p->type == Pout ? "" : " Error");
  for ( c = p->head; c != NULL; c = c->next ) {
    printf("  Cmd #%d: ", ++i);
    prCmd(c);
  }
  printf("End pipe\n");
  prPipe(p->next);
}

int main(int argc, char *argv[])
{
  Pipe p;
  char *hostname="geralt";
  //getting hostname
  char host[1024];
  host[1023] = '\0';
  gethostname(host, 1023);
  //rc file handling
  int stdincopy=dup(0);
  char rc_file[300];
  strcat(rc_file,getenv("HOME"));
  strcat(rc_file,"/");
  strcat(rc_file,".ushrc");
  int in=open(rc_file,O_RDONLY);
  signal_handle(1);
  if(in>=0){
  dup2(in,0);
  close(in);
  while ( 1 ) {
    p = parse();
    if(!strcmp(p->head->args[0], "end"))
      break;
    run_shell(p);
    freePipe(p);
  }
  dup2(stdincopy,0);
  close(stdincopy);
  }
  else
    printf(".ushrc file not found\n");
  while ( 1 ) {
    printf("%s%% ", hostname);
    fflush( stdout );
    p = parse();
    //if(!strcmp(p->head->args[0], "end"))
    //  exit(0);
    if(p==NULL)
      continue;
    run_shell(p);
    //prPipe(p);
    freePipe(p);
  }
}

/*........................ end of main.c ....................................*/
