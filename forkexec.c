int copyImage(u16 child_segment)
{
  // your copyimage function
}

int fork()
{
  int i, pid;  u16 segment;

  PROC *p = kfork(0);   // kfork() but do NOT load any Umode image for child
  if (p == 0)           // kfork failed
    return -1;

  segment = (p->pid+1)*0x1000;
  copyImage(segment);

  // YOUR CODE to make the child runnable in User mode

   /**** Copy file descriptors ****/
   for (i=0; i<NFD; i++){
      p->fd[i] = running->fd[i];
      if (p->fd[i] != 0){
          p->fd[i]->refCount++;
          if (p->fd[i]->mode == READ_PIPE)
              p->fd[i]->pipe_ptr->nreader++;
          if (p->fd[i]->mode == WRITE_PIPE)
	      p->fd[i]->pipe_ptr->nwriter++;
      }
   }

   return(p->pid);
}

int exec(char *filename)
{
  // your exec function
}
