
/*************************************************************************
 - Each semgment is 2 bytes!
 - USP --> umode stack pointer
 - USS --> umode stack
 - a, b, c, and d are the parameters for a syscall and they are stored at the
   high end of the stack. A is the the syscall number, and b,c, and d are any
   other needed arguments.

  usp  1   2   3   4   5   6   7   8   9  10   11   12    13  14  15  16
----------------------------------------------------------------------------
 |uds|ues|udi|usi|ubp|udx|ucx|ubx|uax|upc|ucs|uflag|retPC| a | b | c | d |
----------------------------------------------------------------------------
***************************************************************************/
char *statuses[] = {"FREE", "READY", "RUNNING", "STOPPED", "SLEEP", "ZOMBIE", 0};
/****************** syscall handler in C ***************************/
int kcinth()
{
	int a,b,c,d, r;

	//Load a,b,c,d from the UMODE statck
	a = get_word(running->uss, running->usp+26);	//Load the call number
	b = get_word(running->uss, running->usp+28);	//Arg #1
	c = get_word(running->uss, running->usp+30);	//Arg #2
	d = get_word(running->uss, running->usp+32); 	//Arg #3

	switch(a){
		case 0 : r = kgetpid();		break;
		case 1 : r = kps();		break;
		case 2 : r = kchname(b);	break;
		case 3 : r = kkfork();	break;
		case 4 : r = ktswitch();	break;
		case 5 : r = kkwait(b);		break;
		case 6 : r = kkexit(b);		break;
		case 7 : r = kkgetc();		break;
		case 8 : r = kkputc(b);		break;
		case 9 : r = fork(); 		break;
		case 10 : r = exec(b);		break;
		case 11 : r = kpipe(b); 	break;
		case 12 : r = kpfd();	break;
		case 13 : r = kread_pipe(b,c,d);  break;
		case 14 : r = kwrite_pipe(b,c,d); break;
		case 15 : r = kclose_pipe(b);     break;
		case 16 : r = kshow_pipe(b);	break;
	}

	//==> WRITE CODE to let r be the return value to Umode
	
	//The CPU returns all values from kmode back to umode through the AX
	//register. So it is important that we put any values we want to send 
	//back in that register. In this case we want to put the return value
	//of the kmode function in the AX register so UMODE can see it.
	put_word(r, running->uss, running->usp+16);
}

//============= WRITE C CODE FOR syscall functions ======================

int kgetpid()	//Return the PID of the running process
{
    //WRITE YOUR C code
	return running->pid;
}

int kps()	//Print out all the processes and the inforamtion about them
{
    //WRITE C code to print PROC information
	int i = 0;
	printf("# \tName\tStatus\tPID\tPPID\n");
	for (i = 0; i < NPROC; i++){
		printf("P%d\t%s\t%s\t%d\t%d\n", i, proc[i].name, statuses[proc[i].status], proc[i].pid, proc[i].ppid);
	}
}

int kchname(char *name)	//Rename the processes
{
    	//WRITE C CODE to change running's name string;
	char buf[64];
	char *cp = buf;
	int count = 0; 

	while (count < 32){
		*cp = get_byte(running->uss, name);
		if (*cp == 0) 
			break;
		cp++; //Move currnet pointer forward
		name++; //Move name char pointer forward (where we get the byte from)
		count++;	//Increment size value
	}
	buf[31] = 0;	//Need to set the end to NULL to make sure we don't encounter errors

	printf("Process %d name is now: %s", running->pid, buf);
	
	strcpy(running->name, buf); //Copy the string over from the buf to the proc->name
}

int kkfork()
{
  	//use your kfork() in kernel;
  	//return child pid or -1 to Umode!!!
	PROC *p = kfork("/bin/u1");

	if(p){
		return p->pid;
	}

	return -1;
}

int ktswitch()
{
    return tswitch();	//Run swtich to go to next ready process
}

int kkwait(int *status)
{
  	//use YOUR kwait() in LAB3;
  	//return values to Umode!!!
  	int wstatus, pid;

  	pid = kwait(&wstatus);	//Call kwait
  	put_word(wstatus, running->uss, status);	
	return pid; 
}

int kkexit(int value)
{
    //use your kexit() in LAB3
    //do NOT let P1 die
	return kexit(value);	//Call kexit
}

int kkgetc(){
	return getc();
}

int kkputc(char a){
	return putc(a);
}

int copyImage(u16 pseg, u16 cseg, u16 size){
	u16 i;
	for (i=0; i<size; i++){
		put_word(get_word(pseg, 2*i), cseg, 2*i);
	}
}

int fork()
{
	int i;
	int pid;  
	u16 segment;
	
	//We don't need to load the image becuase we will be copying the
	//image from the parent process. MAGIC!!! :)
	PROC *p = kfork(0);

	//If p is 0, then we know a new child wasn't forked
	//Return -1 and quit
	if (p == 0){
		return -1;
	}

	//The segment should point to its pid+1*0x1000
	segment = (p->pid+1)*0x1000;
	//Because we are using copyImage the UMODE images will be the same
	//ES, DS and CS will be the segment of the parent
	copyImage(running->uss,segment, 32*1024);

	//Need set the uss to where the process is ACUTALLY saved
	p->uss = segment;
	//The usp should point the parents position
	p->usp = running->usp;

	//The two fellows are now in the same bed, we need to fix that. AKA send
	//the child back to it's segment

	//Re set the stack:

	//FRIST: move the copied DS register back
	put_word(segment, segment, p->usp);

	//SECOND: move the copied ES register back
	put_word(segment, segment, p->usp+2);

	//THIRD: set uax register to 0
	put_word(0, segment, p->usp+2*8);

	//FOURTH: set the copied CS register back
	put_word(segment, segment, p->usp*2+10);

	
	//If your fork a child who's parent has a pipe, they will also
	//have the pipe. This increments the ref counts of the pipes when
	//new procs are spawned.
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
	//Return the child pid
	return p->pid;
}


int exec(int name)
{
	int i, li, length = 0;
	char filename[32];
	char letter;
	int count = 0; 
	
	//We need to know where to load the new executable (current segment)
	u16 segment = running->uss;

	//Copy the name over from UMODE (NOTE: We cannot directly copy it)
	for (li = 0; li < 32; li++){
		letter = get_byte(running->uss, name+li);

		filename[li] = letter;

		if (get_byte(running->uss, name+li) == 0){	
			break;
		}

	}
	
	//Try to load the file, if it fails return -1
	if (load(filename, segment) == 0){
		return -1;
	}
	
	/*********************************************************************************
		After loading the new Umode image, fix up the ustack contents to make the
     		process execute from virtual address 0 when it returns to Umode. Refer to 
     		the diagram again:

		     (LOW)  uSP                                | by INT 80  |   HIGH
		     ---------------------------------------------------------------------
			   |uDS|uES| di| si| bp| dx| cx| bx| ax|uPC|uCS|flag| XXXXXX
		     ---------------------------------------------------------------------
			    -12 -11 -10  -9  -8  -7  -6  -5  -4  -3  -2  -1 | 0 

		(a). re-establish ustack to the very high end of the segment.
		(b). "pretend" it had done  INT 80  from (virtual address) 0: 
		     (c). fill in uCS, uDS, uES in ustack
		     (d). execute from VA=0 ==> uPC=0, uFLAG=0x0200, 
		                                all other registers = 0;
		     (e). fix proc.uSP to point at the current ustack TOP (-24)
	*********************************************************************************/


	//Set all of the uss registers to 0
	//This wil allow it to execute from VA 0 when back in UMODE
	//NOTE: Offset of 2 for registers
	for (i=1; i<=12; i++){
		put_word(0, segment, -2*i);
	}
	
	//Point the stack pointer back to the top
	running->usp = -24;
	/*******************************************************
	* -1 -2 -3 -4 -5 -6 -7 -8 -9 -10 -11 -12 ustack layout *
	* flag uCS uPC ax bx cx dx bp si di uES uDS            *
	*******************************************************/
	put_word(segment, segment, -2*12); //uDS = segment
	put_word(segment, segment, -2*11); //uES = segment
	put_word(segment, segment, -2*2); // uCS = segment
	put_word(0x0200, segment, -2*1); // flag = 0x0200
}

int kpfd(){
	pfd();
	return 0;
}

int kkpipe(int *pd){
	return kpipe(pd);
}

int kclose_pipe(int b){
	close_pipe(b);

	return 0;
}

int kread_pipe(int fd, char *buf, int n){
	char sBuf[32];
	char *cp;
	int index=0;

	read_pipe(fd, sBuf, n);
	//Write the string to umode 
	
	cp = sBuf;

	while (*cp){
		put_word(*cp, running->uss, buf+index); 
		cp++;
		index++;
	}

	put_word(0, running->uss, buf+index);

	return 0;
}

int kwrite_pipe(int fd, char *buf, int n){
	char sBuf[32];
	//Get the buf from umode
	int count = 0; 
	char *cp = sBuf;

	while (count < 32){
		*cp = get_byte(running->uss, buf);
		if (*cp == 0) 
			break;
		cp++; //Move currnet pointer forward
		buf++; //Move name char pointer forward (where we get the byte from)
		count++;	//Increment size value
	}
	sBuf[32] = 0; 

	printf("sbuf: %s\n\n", sBuf);

	write_pipe(fd, sBuf, n);

	return 0;
}

int kshow_pipe(int pNum){
	PIPE *p = &pipe[pNum];
	show_pipe(p);
}
