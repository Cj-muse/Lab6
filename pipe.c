#include "header.h"
show_pipe(PIPE *p)
{
   int i, j;
   // print pipe information
   printf("------------ PIPE CONTENETS ------------\n");
   printf("head: %d\n", p->head);
   printf("tail: %d\n", p->tail);
   printf("data: %d\n", p->data);
   printf("room: %d\n", p->room);
   printf("nreader: %d\n", p->nreader);
   printf("nwriter: %d\n", p->nwriter);
   printf("busy: %d\n", p->busy);
   printf("buffer: %s\n", p->buf);
   printf("----------------------------------------\n");
}

char *MODE[ ]={"READ_PIPE ","WRITE_PIPE"};

int pfd()
{
  int i = 0;

  // print running process' opened file descriptors
  printf("--------Proc %dOpen File Decsriptors---------\n", running->pid);
  for(i=0; running->fd[i]; i++)
  {    printf("fd[%d]: %d \n", i, running->fd[i]);  }

  if (0 == i)  { printf("NONE\n");}
  printf("---------------------------------------------\n");
}
//============================================================


int read_pipe(int fd, char *buf, int n)
{
  // your code for read_pipe()
}

int write_pipe(int fd, char *buf, int n)
{
  // your code for write_pipe()
}

int kpipe(int pd[2])
{
  int i = 0;
  PIPE *p;
  OFT *readFT, *writeFT;

  // create a pipe; fill pd[0] pd[1] (in USER mode!!!) with descriptors
  initPipe(p);
  initOFT(readFT, READ_PIPE, p);
  initOFT(writeFT, WRITE_PIPE, p);

  show_pipe(p);
  getc();
  //  Allocate 2 free entries in the PROC.fd[] array,
  for (i=0; i < NFD-1; i++)
  {
      if (running->fd[i] == 0 && running->fd[i+1] == 0)
      {
          running->fd[i]   = readFT;
          running->fd[i+1] = writeFT;
          break;
      }
  }

  // set indicies of running procs fd's to pd[]
  pd[0] = i;
  pd[1] = i+1;

  printf("returning from kpipe\n");

  return 0;
}

int close_pipe(int fd)
{
  OFT *op; PIPE *pp;

  printf("proc %d close_pipe: fd=%d\n", running->pid, fd);

  op = running->fd[fd];
  running->fd[fd] = 0;                 // clear fd[fd] entry

  if (op->mode == READ_PIPE){
      pp = op->pipe_ptr;
      pp->nreader--;                   // dec n reader by 1

      if (--op->refCount == 0){        // last reader
	if (pp->nwriter <= 0){         // no more writers
	     pp->busy = 0;             // free the pipe
             return;
        }
      }
      //was wakeup() not sure if necessary
      kwakeup(&pp->room);               // wakeup any WRITER on pipe
      return;
  }

  // YOUR CODE for the WRITE_PIPE case:

}

int initPipe(PIPE *p)
{
  p->head = p->tail = p->data = 0;
  p->nwriter = p->nreader = 1;
  p->room = PSIZE;
  p->busy = 0;
  return 1;
}

int initOFT(OFT *t, int mode, PIPE *p)
{
  t->mode = mode;
  t->refCount = 1;
  t->pipe_ptr = p;
  return 1;
}
