#include "header.h"
show_pipe(PIPE *p)
{
   int i, j;
   // print pipe information
   printf("------------ PIPE CONTENTS ------------\n");
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
  {
    printf("fd[%d]: %d \n", i, running->fd[i]);
    printf("fd->mode: %d\n", running->fd[i]->mode);
    //printf("fd->pipe->busy: %d\n", running->fd[i]->(struct *pipe_ptr));
  }
  if (0 == i) {printf("NONE\n");}
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

  printf("creating pipe\n");
  for (i=0; i<NPIPE; i++)
  {
    if (pipe[i].busy == 0)
    break;
  }
  pipe[i].busy = 1;
  p = &pipe[i];

  // create a pipe; fill pd[0] pd[1] (in USER mode!!!) with descriptors
  initPipe(p);
  initOFT(readFT, READ_PIPE, p);
  initOFT(writeFT, WRITE_PIPE, p);

  show_pipe(p);

  //  Allocate 2 free entries in the PROC.fd[] array,
  for (i=0; i < NFD-1; i++)
  {
      if (running->fd[i] == 0 && running->fd[i+1] == 0)
      {
          running->fd[i]   = readFT;
          running->fd[i+1] = writeFT;
          //put_word(i, running->uss, pd++);
          //put_word(i+1, running->uss, pd);
          break;
      }
  }

  // set indicies of running procs fd's to pd[]
  pd[0] = i;
  pd[1] = i+1;

  /* fill user pipe[] array with i, i+1 */
  put_word(i, running->uss, pd); put_word(i+1, running->uss, pd+1);
  //put_word(i, running->uss, pd++); put_word(i+1, running->uss, pd);
  printf("do_pipe : file descriptors = [%d %d]\n", i, i+1);
  printf("do_pipe : file descriptors = [%d %d]\n", pd[0], pd[1]);

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
  return 1;
}

int initOFT(OFT *t, int mode, PIPE *p)
{
  int i = 0;
  for (i=0; i<NOFT; i++){
      if (oft[i].refCount == 0) break;
  }
  t = &oft[i];
  t->mode = mode;
  t->refCount = 1;
  t->pipe_ptr = p;
  return 1;
}
