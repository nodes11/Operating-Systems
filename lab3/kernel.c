// added scheduling functions in MTX4.5
int reschedule()
{
	PROC *p, *tempQ = 0;	//Create temp procs

	while ( (p=dequeue(&readyQueue)) ){ // reorder readyQueue
		enqueue(&tempQ, p);
	}

	readyQueue = tempQ;
	rflag = 0; // global reschedule flag
	
	if (running->priority < readyQueue->priority)
		rflag = 1;
}
