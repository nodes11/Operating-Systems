int ksleep(int event){
	running->event = event; // record event in PROC.event
	running->status = SLEEP; // change status to SLEEP

	enqueue(&sleepList, running);
	nproc--;

	tswitch();
}

int kwakeup(int event){
	int i; PROC *p;

	for (i=1; i<NPROC; i++){ // not applicable to P0
		p = &proc[i];
		if (p->status == SLEEP && p->event == event){
			removeSleep(&sleepList, p->event);

			p->event = 0; // cancel PROC’s event
			p->status = READY; // make it ready to run again
						
			enqueue(&readyQueue, p);
		}
	}
}

int kexit(int exitValue)
{
 	int i, wakeupP1 = 0;
	PROC *p;

	printf("NPROC=%d\nrunning->pid:%d\n", nproc, running->pid);
 	if (running->pid==1 && nproc>2){ //If we still have child PROCS
		// nproc = number of active PROCs
 		printf("other procs still exist, P1 can't die yet\n");
 		return -1;
 	}

 	/* send children (dead or alive) to P1's orphanage */
	for (i = 1; i < NPROC; i++){
 		p = &proc[i];
		
		//For all PROCS that aren't FREE or whose parent is running,
		//set their new parent as P1
		if (p->status != FREE && p->ppid == running->pid){
			p->ppid = 1;
			p->parent = &proc[1];
			wakeupP1++;
 		}
 	}	

	/* record exitValue and become a ZOMBIE */
 	running->exitCode = exitValue;
 	running->status = ZOMBIE;
	
	//decrement number of procs
	nproc--;

	/* wakeup parent and also P1 if necessary */
 	kwakeup(running->parent); // parent sleeps on its PROC address
 	if (wakeupP1)
		kwakeup(&proc[1]);

 	tswitch(); // give up CPU
}

int kwait(int *status) // wait for ZOMBIE child
{
	PROC *p; int i, hasChild = 0;

	while(1){ // search PROCs for a child
 		for (i=1; i<NPROC; i++){ // exclude P0
			
			p = &proc[i];
			
			if (p->status != FREE && p->ppid == running->pid){
				hasChild = 1; // has child flag
 				if (p->status == ZOMBIE){ // lay the dead child to rest
					*status = p->exitCode; // collect its exitCode
					p->status = FREE; // free its PROC
					put_proc(&freeList, p); // to freeList
					nproc--; // once less processes
					return(p->pid); // return its pid
				}
			}	
		}

		if (!hasChild)
			return -1; // no child, return ERROR
	
		ksleep(running); // still has kids alive: sleep on PROC address
 	}
}
