int show_pipe(PIPE *p)
{
   int i, j;
   printf("------------ PIPE CONTENTS ------------\n");     
   // print pipe information
   printf("BUF:\t%s\n", p->buf);
   printf("HEAD:\t%d\n", p->head);
   printf("TAIL:\t%d\n", p->tail);
   printf("ROOM:\t%d\n", p->room);
   printf("DATA:\t%d\n", p->data);
   printf("BUSY:\t%d\n", p->busy);
   printf("READERS:\t%d\n", p->nreader);
   printf("WRITERS:\t%d\n", p->nwriter);
   printf("\n----------------------------------------\n");
}

char *MODE[2]={"READ","WRITE"};

//Will print out all open file descriptors
int pfd()
{
	PROC *p = running;
	int i;

	//Index		Mode	Ref Count
	printf("#\tMode\tRef Count\n");
	for (i = 0; i < NFD; i++){
		//If it has a ref count greater than 1, then it's being used
		if (p->fd[i]->refCount > 0){
			if (p->fd[i]->mode == 4){
				printf("%d.\t%s\t%d\n", i, MODE[0], p->fd[i]->refCount);
			}
			else if (p->fd[i]->mode == 5){
				printf("%d.\t%s\t%d\n", i, MODE[1], p->fd[i]->refCount);
			}
			else{
				printf("%d.\t%d\t%d\n", i, p->fd[i]->mode, p->fd[i]->refCount);
			}
		}
	}
}
//============================================================


int read_pipe(int fd, char *buf, int n)
{
  // your code for read_pipe()
	int readIn = 0;

	//We want to read from the pipe to get the contents
	PIPE *p = running->fd[fd]->pipe_ptr;
	
	//Where we're writing to
	char *bufPtr = buf;

	int i = p->head, 
		j = 0,
		newI;
	
	printf("N: %d\n", n);

	//Make sure they actually want to read someting...
	if (n <= 0){
		return 0;
	}

	//printf("READING %d BYTES\n\n", n);
	show_pipe(p);
	//Make sure we have a valid pipe
	if (p){
		while(n){
			newI = 0;
			i = 0;

			//Read from the pipe (max of 10 bytes)
			while(p->data){
				//If we've read in the requested amount
				if (readIn == n)
					break;

				printf("READING %d\n", n);

				//Read the word from the pipe into the buf
				put_word(p->buf[i], running->uss, bufPtr++);
				p->buf[i] = 0;
				i++;
				//Move counters
				readIn++;
				p->data--;
				p->head--;
				p->room++;
			}

			//NEED TO SHIFT!!!!
			printf("i is %d!!!\n", i);
			while(p->buf[i]){
				p->buf[newI] = p->buf[i];
				i++;
				newI++;
			}
			
			//If we've read data, wake up others in the room
			//Return the amount we've read
			if (readIn){
				kwakeup(&p->room);
				show_pipe(p);
				return readIn;
			}
			
			//If there is still a writer, wake them up
			//and see if they can write. Then sleep for for data
			if (p->nwriter){
				printf("waking others!!!\n\n");
				kwakeup(&p->room);
				printf("sleeping for data!!!\n\n");
				ksleep(&p->data);
				continue;
			}
			show_pipe(p);
			return 0;

		}
	}

}

int write_pipe(int fd, char *buf, int n)
{ 
	int r = 0, i, src=0, newI = 0;
	//validate fd; from fd, get OFT and pipe pointer p;
	PIPE *p = running->fd[fd]->pipe_ptr;

	if (n <= 0){
		return 0;
	}

	i = p->head;

	printf("Starting i: %d, starting src: %d\n\n",i, src);

	while (n){
		src = 0;
		i = p->head;

		if (!p->nreader) // no more readers
			kexit(BROKEN_PIPE); // BROKEN_PIPE errori

		printf("buf to write: %s, src: %d\n", buf, src);

		while(p->room > 0){
			//write a byte from buf to pipe;
			p->buf[i] = buf[src];
			buf[src] = 0;
			i++; src++;
			r++; p->data++; p->room--; n--; p->head++;
			if (n==0)
				break;
		}


		//If we write more than 10 bytes then we will need to keep the
		//left over bytes and they will get put into the pipe once we switch
		//back into the writing process after the reading completes and the child
		//finshes
		if (buf[src]){
			newI = 0;
			while(buf[src] != 0){
				printf("buf[src] = %c\n", buf[src]);
				buf[newI] = buf[src];
				printf("buf[newI] = %c\n\n", buf[newI]);
				buf[src] = 0;
				src++;
				newI++;
			}

			//Print the pipe
			show_pipe(p);
		}
		printf("waking up readers\n\n");	
		kwakeup(&p->data); // wakeup ALL readers, if any.
		if (n==0)
			return r; // finished writing n bytes
		// still has data to write but pipe has no room
		ksleep(&p->room); // sleep for room
	}
}


OFT *makeOFT(int mode){
	int i;

	//Inialize a file descriptor
	for (i = 0; i < NOFT; i++){
		if (!oft[i].refCount){
			oft[i].mode = mode;
			oft[i].pipe_ptr = 0;
			oft[i].refCount = 1;
			return &oft[i];
		}
	}
}

//Allocate a new pipe for use
PIPE *makePipe(){
	int i;

	//Initialize all pipe contents
	for (i = 0; i < NPIPE; i++){
		if (pipe[i].busy != 1){
			pipe[i].room = PSIZE;

			pipe[i].head = 0;
			pipe[i].tail = 0;
			pipe[i].data = 0;
			
			pipe[i].nreader = 0;
			pipe[i].nwriter = 0;

			pipe[i].busy = 1;
			return &pipe[i];
		}
	}
}

int kpipe(int *pd)
{
	// create a pipe; fill pd[0] pd[1] (in USER mode!!!) with descriptors
	int readP = 9999,
		writeP = 9999,
		i = 0;

	//Allocate
	PIPE *newPipe = makePipe();
	newPipe->nwriter = 1;
	newPipe->nreader = 1;

	for (i = 0; i < NFD; i++){
		//Write and write sides have been set
		if (readP != 9999 && writeP != 9999)
			break;

		if (running->fd[i] == 0){
			if (readP == 9999){	//Set the read
				printf("\nSetting the read pipe at i=%d\n",i);
				running->fd[i] = makeOFT(READ_PIPE);
				running->fd[i]->pipe_ptr = newPipe;
				readP = i;
			}
			else if (writeP == 9999){	//Set the write
				printf("setting the write pipe at i=%d\n\n", i);
				running->fd[i] = makeOFT(WRITE_PIPE);
				running->fd[i]->pipe_ptr = newPipe;
				writeP = i;
				break;
			}
		}
	}


	//Write the read and write sides to the umode segment
	put_word(readP, running->uss, pd);
	put_word(writeP, running->uss, pd+1);
}

int close_pipe(int fd)
{
	OFT *op; PIPE *pp;

	//Print the pipe you're closing
	printf("proc %d close_pipe: fd=%d\n", running->pid, fd);

	//Set OFT pointer
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
      	kwakeup(&pp->room);               // wakeup any WRITER on pipe 
		return;
	}
  
	// YOUR CODE for the WRITE_PIPE case:
	else if (op->mode == WRITE_PIPE){
		pp = op->pipe_ptr;
		pp->nwriter--;

		if (--op->refCount == 0){
			if (pp->nreader <= 0){
				pp->busy = 0;
				return;
			}
		}
		kwakeup(&pp->room);
		return;
	}

}
