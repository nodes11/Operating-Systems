
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
		case 0 : r = kgetpid();        break;
		case 1 : r = kps();            break;
		case 2 : r = kchname(b);       break;
		case 3 : r = kkfork();         break;
		case 4 : r = ktswitch();       break;
		case 5 : r = kkwait(b);        break;
		case 6 : r = kkexit(b);        break;
		case 7 : r = kkgetc();		   break;
		case 8 : r = kkputc(b);		   break;

		case 99: kkexit(b);            break;
		default: printf("invalid syscall # : %d\n", a); 
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

  	pid = kwait(&wstatus);	//Call kwait and wait until the child dies (ZOMBIE)
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
