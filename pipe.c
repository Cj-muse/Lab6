#include "header.h"
show_pipe(PIPE *p)
{
   int i, j;
   //validate pipe
   if (!pipe)
   {
     printf("show_pipe(): Invalid Pipe\n");
   }
   // print pipe information
   printf("------------ PIPE %d CONTENTS ------------\n", p);
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

//char *MODE[ ]={"READ_PIPE ","WRITE_PIPE"};

int pfd()
{
  int i = 0;

  // print running process' opened file descriptors
  printf("--------Proc %dOpen File Decsriptors---------\n", running->pid);

  for(i=0; running->fd[i] != 0; i++)
  {
    printf("fd[%d]: \n",i);
    printf("mode:     %d\n",   running->fd[i]->mode);
    printf("refCount: %d\n",   running->fd[i]->refCount);
    //printf("fd->pipe->busy: %d\n", running->fd[i]->(struct *pipe_ptr));
  }
  if (0 == i) {printf("NONE\n");}

  printf("---------------------------------------------\n");
}

int read_pipe(int fd, char *buf, int n)
{
  // your code for read_pipe()
  int r = 0;
  OFT *oft = running->fd[fd];
  PIPE *p = oft->pipe_ptr;
  if (n<=0)
  return 0;

  //perform checks on pipe ptr and open file table
  if (!oft) {
    printf("read_pipe: not a valid fd\n");
    return -1;
  }
  if (!p) {
    printf("read_pipe: not a valid pipe_ptr\n");
    return -1;
  }

  printf("ReadPipe: \n");
  show_pipe(p);
  printf("oft %d\n", oft);
  printf("oft->refCount: %d\n", oft->refCount);
  printf("oft->mode: %d\n", oft->mode);

  //validate fd; from fd, get OFT and pipe pointer p;
  while(n)
  {
    while(p->data)
    {//read a byte form pipe to buf;
      buf[r] = p->buf[r];
      n--; r++; p->data--; p->room++;
      if (n==0)
      break;
    }
    if (r){ // has read some data
      kwakeup(p->room);
      return r;
    }
    // pipe has no data
    if (p->nwriter){ // if pipe still has writer
      kwakeup(p->room); // wakeup ALL writers, if any.
      ksleep(p->data); // sleep for data
      continue;
    }
    // pipe has no writer and no data
    return 0;
  }
}

int write_pipe(int fd, char *buf, int n)
{
  // your code for write_pipe()
  int r = 0;
  OFT *oft = running->fd[fd];
  PIPE *p = oft->pipe_ptr;

  if (n<=0)
  return 0;

  //perform checks on pipe ptr and open file table
  if (!oft) {
    printf("write_pipe: not a valid fd\n");
    return -1;
  }
  if (!p) {
    printf("write_pipe: not a valid pipe_ptr\n");
    return -1;
  }

  //validate fd; from fd, get OFT and pipe pointer p;
  while (n)
  {
    if (!p->nreader) // no more readers
      kexit(BROKEN_PIPE); // BROKEN_PIPE error
    while(p->room)
    {
      //write a byte from buf to pipe;
      r++; p->data++; p->room--; n--;
      if (n==0)
      break;
    }

    kwakeup(p->data); // wakeup ALL readers, if any.
    if (n==0)
    return r; // finished writing n bytes

    // still has data to write but pipe has no room
    ksleep(p->room); // sleep for room
  }
}

int kpipe(int pd[2])
{
  int i = 0;
  PIPE *p;
  OFT *readFT, *writeFT;

  // create a pipe; fill pd[0] pd[1] (in USER mode!!!) with descriptors
  p = initPipe();
  readFT = initOFT(READ_PIPE, p);
  writeFT = initOFT(WRITE_PIPE, p);
  readFT->pipe_ptr = p;
  writeFT->pipe_ptr = p;

  //  Allocate 2 free entries in the PROC.fd[] array,
  for (i=0; i < NFD-1; i++)
  {
    printf("allocating file descriptors\n");
      if (running->fd[i] == 0 && running->fd[i+1] == 0)
      {
          running->fd[i]   = readFT;
          running->fd[i+1] = writeFT;
          break;
      }
  }

  printf("allocation successfull\n");
  // set indicies of running procs fd's to pd[]
  pd[0] = i;
  pd[1] = i+1;

  /* fill user pipe[] array with i, i+1 */
  //printf("p = %d\n", p);
  put_word(i, running->uss, pd); put_word(i+1, running->uss, pd+1);
  show_pipe(p);

  printf("returning from kpipe\n");
  //getc();
  return 0;
}

int close_pipe(int fd)
{
  OFT *op; PIPE *pp;

  printf("proc %d close_pipe: fd=%d\n", running->pid, fd);

  op = running->fd[fd];
  if (!op) // validate oft pointer
  {
    printf("sys: close_pipe(): invalid file descriptor\n");
    return -1;
  }

  running->fd[fd] = 0;                 // clear fd[fd] entry
  pp = op->pipe_ptr;

  if (op->mode == READ_PIPE){
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
  else
  { // YOUR CODE for the WRITE_PIPE case:
    pp->nwriter--;                   // dec n writers by 1

    if (--op->refCount == 0){        // last  writer
      if (pp->nreader <= 0){         // no more readers
           pp->busy = 0;             // free the pipe
           return;
      }
    }
    kwakeup(&pp->data);           // wakeup any READERS on pipe
    return;
  }
}

PIPE *initPipe()
{
  int i = 0;
  //PIPE *p;

  printf("creating pipe\n");
  for (i=0; i<NPIPE; i++)
  {
    printf("pipe[%d]: %d\n",i,pipe[i]);
    if (pipe[i].busy == 0)
    break;
  }
  pipe[i].busy = 1;
  //p = &pipe[i];
  //printf("p = %d\n",p);

  pipe[i].head = pipe[i].tail = pipe[i].data = 0;
  pipe[i].nwriter = pipe[i].nreader = 1;
  pipe[i].room = PSIZE;
  return &pipe[i];
}

OFT *initOFT(int mode, PIPE *p)
{
  int i = 0;
  //OFT *t;

  for (i=0; i<NOFT; i++){
      if (oft[i].refCount == 0) break;
  }
  //t = &oft[i];
  printf("in initoft, mode = %d\n", mode);
  oft[i].mode = mode;
  oft[i].refCount = 1;
  //oft[i].pipe_ptr = p;
  return &oft[i];
}
