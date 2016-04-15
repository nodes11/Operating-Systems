/*********** MTX4.2 kernel t.c file **********************/
#define NPROC 9
#define SSIZE 1024

/******* PROC status ********/
#define FREE 0
#define READY 1
#define STOP 2
#define DEAD 3

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef struct proc{
    struct proc *next;
    int *ksp;
    int pid; // add pid for identify the proc
    int ppid; // parent pid;
    int status; // status = FREE|READY|STOPPED|DEAD, etc
    int priority; // scheduling priority
    int kstack[SSIZE]; // proc stack area
} PROC;

PROC proc[NPROC], *running, *freeList, *readyQueue;

int procSize = sizeof(PROC);

#include "bio.c" // include I/O functions based on getc()/putc()

int body()
{
    char c;
    printf("\nproc %d starts from body()\n", running->pid);
    while(1) {
        printf("freeList ");// optional: show the freeList
        printList(freeList); 
		printf("\nreadyQueue "); // show the readyQueue
        printList(readyQueue); 
		
	printf("\nproc %d running: parent=%d\n",running->pid, running->ppid);
	putc('\n');
        printf("enter a char [s|f|q] : ");
        c = getc(); 
	printf("%c\n", c);

        switch(c){
            case 'f' : do_kfork(); break;
            case 's' : do_tswitch(); break;
		case 'q' : quit(); break;
        }
    }
}


//adds proc to start of free list
put_proc(PROC *p)
{
		//create proc to place at start of free list 
		PROC *currProc = freeList;

		//Place the PROC and set it's next
		p->next = currProc;
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

//gets a free item from the list
PROC *get_proc()
{
		//PROC *cp = freeList;

		if (freeList != 0){
			return dequeue(&freeList);
		}

		return 0;
}

//print the list
printList(PROC *queue)
{
		PROC *currNode = queue;

		printf("printing:");

		while (currNode){
			printf("[%d, %d]", currNode->pid, currNode->priority);
			currNode = currNode->next;

			if (currNode){
				printf(" --> ");
			}
		}

		printf(" --> NULL");
}


PROC *kfork() // create a child process, begin from body()
{
	int i;
    PROC *p = get_proc();

    if (p == 0) {
        printf("no more PROC, kfork() failed\n");
        return 0;
    }

    p->status = READY;
    p->priority = 1; // priority = 1 for all proc except P0
    p->ppid = running->pid; // parent = running

	
    /* initialize new proc's kstack[ ] */
   	for (i=1; i<10; i++){ // saved CPU registers
        p->kstack[SSIZE-i] = 0; // all 0's
	}
    
    p->kstack[SSIZE-1] = (int)body; // resume point=address of body()
    p->ksp = &p->kstack[SSIZE-9]; // proc saved sp

    enqueue(&readyQueue, p); // enter p into readyQueue by priority

    return p; // return child PROC pointer
}

quit(){
	running->status = DEAD;
	tswitch();
}

int init()
{
    PROC *p; int i;
    printf("init ....\n\n");
    
    for (i=0; i<NPROC; i++) { // initialize all procs
		p = &proc[i];
        p->pid = i;
        p->status = FREE;
        p->priority = 0;
        p->next = &proc[i+1];
    }

    proc[NPROC-1].next = 0;
    freeList = &proc[0]; // all procs are in freeList
    readyQueue = 0;
    
    /**** create P0 as running ******/
    p = get_proc(); // allocate a PROC from freeList
    p->ppid = 0; // P0’s parent is itself
    p->status = READY;
    running = p; // P0 is now running
}

int scheduler()
{
    if (running->status == READY) // if running is still READY
        enqueue(&readyQueue, running); // enter it into readyQueue
    running = dequeue(&readyQueue); // new running
}

main()
{
    printf("\nMTX has started\n\n");
    init(); // initialize and create P0 as running

	// TODO: print initialization complete
	printf("Initialization has finished\n\n");

    kfork(); // P0 creates child P1

	// TODO: print P0 switching (could go right inside the while)
    printf("Fork completed\n\n"); 	

    while(1) { // P0 switches if readyQueue not empty
        if (readyQueue)
            tswitch();
    }
}

/*************** kernel command functions ****************/
int do_kfork( )
{
    PROC *p = kfork();
    if (p == 0) { 
        printf("kfork failed\n"); return -1; 
    }
    printf("PROC %d kfork a child %d\n", running->pid, p->pid);
    return p->pid;
}

int do_tswitch(){
	tswitch(); 
}
