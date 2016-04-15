//adds proc to start of free list
put_proc(PROC **freeList, PROC *p)
{
	printf("IN PUT PROC!\n\nPID: %d\n\n",p->pid);
	//Place the PROC and set it's next
	p->next = *freeList;
	*freeList = p;
}

//adds proc to enqueue based on priority
enqueue(PROC **queue, PROC *p)
{
		//The queue will always pass the head of itself. So enqueue's will be O(n) and
		//dequeue's will be O(1)
		PROC *currProc = *queue;
		PROC *trailer = *queue;

		
		//check to make sure p exists
		if (p){
			if (*queue == 0){	//If the queue is empty...
				*queue = p;
				p->next = 0;
			}
			else{	//Otherwise..
				//If the proroty is greater than the front
				if (p->priority > (*queue)->priority){
					p->next = *queue;
					(*queue) = p;
				}
				else{
					//if the prority is less than or equal to the front, we want to move through the list
					//Move until we reach the proper place...
					while ((currProc) && (currProc->priority >= p->priority))
					{
						trailer = currProc;
						currProc = currProc->next;
					}	
				
					//Insertion in the middle of the queue
					//
					//we have p to insert
					//
					//pre: trailer --> curr
					//post: trailer->p->currProc
					
					trailer->next = p;
					p->next = currProc;
				}
			}
		}
	
}

//removes frpm front of queue
dequeue(PROC **queue){
	//Point head at the front of the queue
	PROC *head = *queue;

	//If the queue is not empty move it forward
	if (*queue != 0){
		*queue = (*queue)->next;
		return head;
	}

	//Return the head
	return head;
}

//removes frpm front of queue
removeSleep(PROC **queue, int event){
	//The queue will always pass the head of itself. So enqueue's will be O(n) and
	//dequeue's will be O(1)
	PROC *currProc = *queue;
	PROC *trailer = 0;

	//check to make sure p exists
	if (*queue){
		while (currProc){
			if (currProc->event == event){
				if (trailer != 0){
					trailer->next = currProc->next;
					currProc->next = 0;
					return currProc;
				}
				else{
					return dequeue(&sleepList);
				}

			}

			trailer = currProc;
			currProc = currProc->next;
		}
	}
}

//gets a free item from the list
PROC *get_proc(){
	//PROC *cp = freeList;
	if (freeList != 0){
		return dequeue(&freeList);
	}

	return 0;
}

//print the list
printList(PROC *queue, int mode){
		PROC *currNode = queue;
		int flag = 0;

		while (currNode){
			flag = 1;
			if (mode == 1){	//Sleep
				printf("[%d, %d, %d]", currNode->pid, currNode->priority, currNode->event);
			}
			else{
				printf("[%d, %d]", currNode->pid, currNode->priority);
			}
			currNode = currNode->next;

			if (currNode){
				printf("-->");
			}
		}

		if (flag == 1)
			printf("-->NULL");
		else
			printf("NULL");
}

