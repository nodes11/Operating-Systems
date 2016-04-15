#define NPROC    9
#define SSIZE 1024

/******* PROC status ********/
#define FREE     0
#define READY    1
#define RUNNING  2
#define STOPPED  3
#define SLEEP    4
#define ZOMBIE   5

//Defined types used in bio.c
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

//The basic PROC structure. Makes up a node for the list/queue/
typedef struct proc{
    struct proc *next;

    int    *ksp;

    int    pid;                // add pid for identify the proc
    int    status;             // status = FREE|READY|RUNNING|SLEEP|ZOMBIE    
    int    ppid;               // parent pid
    struct proc *parent;
    int    priority;
    int    event;
    int    exitCode;

    int    kstack[SSIZE];      // per proc stack area
}PROC;

//Define our lists and queues
PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;

//Keeps track of proc size, number of ready-procs, and...
int procSize = sizeof(PROC);
int nproc = 0, rflag;

//Allows us to change the text color (BX register)
extern int color;

#include "bio.c" // include I/O functions based on getc()/putc()
#include "wait.c"
#include "kernel.c"

int body()
{
	char c;
    printf("\nproc %d starts from body()\n", running->pid);

    while(1) {
		color = running->pid + 1;	//Change the color depending on the running process

		printf("\n---------------------------------------------------------------\n\n");
		
		printf("freeList: ");// optional: show the freeList
		printList(freeList,0); 
		printf("\n");

		printf("\nreadyQueue: "); // show the readyQueue
		printList(readyQueue,0);
		printf("\n");

		printf("\nsleepList: "); 
		printList(sleepList,1);
		printf("\n");
		
		printf("\nproc %d running: parent=%d\n",running->pid, running->ppid);
		putc('\n');

		printf("Enter a char [s|f|q z|a|w] : ");
		c = getc(); 
		printf("%c\n", c);
	
        switch(c){
            case 'f' : do_kfork(); break;
            case 's' : do_tswitch(); break;
			case 'q' : do_exit(); break;
 			case 'z' : do_sleep(); break;
 			case 'a' : do_wakeup(); break;
			case 'w' : do_wait(); break;
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
					printf("first item\n\n");
					return dequeue(&sleepList);
				}

			}

			trailer = currProc;
			currProc = currProc->next;
		}
	}
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
printList(PROC *queue, int isSleep)
{
		PROC *currNode = queue;
		int flag = 0;

		while (currNode){
			flag = 1;
			if (isSleep == 1){
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


PROC *kfork() // create a child process, begin from body()
{
	int i;
    PROC *p = get_proc();	//Get a PROC from the freelist

    if (p == 0) {	//Make sure there was a PROC to get
        printf("no more PROC, kfork() failed\n");
        return 0;
    }

    p->status = READY;	//Set it's status to ready
    p->priority = 1; // priority = 1 for all proc except P0
    p->ppid = running->pid; // parent = running

	
    /* initialize new proc's kstack[ ] */
   	for (i=1; i<10; i++){ // saved CPU registers
        p->kstack[SSIZE-i] = 0; // Set all registers to null
	}
    
    p->kstack[SSIZE-1] = (int)body; // Set the return address!!!
    p->ksp = &p->kstack[SSIZE-9]; // proc saved sp

    enqueue(&readyQueue, p); // enter p into readyQueue by priority

	nproc++;	//We have created a new child process, incremnet proc count

    return p; // return child PROC pointer
}

int init()
{
    PROC *p;	//Create a temporary process to add to the PROC array
    int i;

    printf("Initializing...\n");

    for (i=0; i<NPROC; i++){   	// initialize all procs
         p = &proc[i];			// point to an index in an array
         p->pid = i;			// set pid
         p->status = FREE;		// set status
         p->priority = 0;     	// set priority
         p->next = &proc[i+1];	// point to next
    }

    freeList = &proc[0];      	// all procs are in freeList
    proc[NPROC-1].next = 0;	  	// point last item next to null
    readyQueue = sleepList = 0;	// set sleeplist and readyQueue to null

    /**** create P0 as running ******/
    p = get_proc(&freeList);	// set P0
    p->status = READY;			// set status as ready
    running = p;				// make running proc

    nproc++;                 	// number of active PROCs 
}

int scheduler()
{
    if (running->status == READY) // if running is still READY
        enqueue(&readyQueue, running); // enter it into readyQueue
    running = dequeue(&readyQueue); // new running
}

int main()
{
    printf("\n\nMTX starts in main()\n\n");
    
	init();      // initialize the system and create P0 as running
	printf("Initialization complete\n\n");
	
	printf("Forking...\n");
    kfork();     // Parent P0 kfork() child P1
	printf("Forking complete\n\n");

    while(1){
		printf("proc 0 running\n\n");

		if (nproc<2 && proc[1].status != READY)
			printf("no runable process, system halts\n\n");
		
		while(!readyQueue);
		
		printf("proc 0 switch process\n");
		tswitch();   // P0 switch to run P1
	}
}

/*************** kernel command functions ****************/
int do_kfork( )
{
    PROC *p = kfork();
    if (p == 0) { 
        printf("kfork failed\n"); return -1; 
    }
    printf("PROC %d kfork a child %d\n\n", running->pid, p->pid);
    return p->pid;
}

int do_tswitch(){
	tswitch(); 
}

int do_exit(){
	//get sleep event
	char ent[64];

	printf("\nEnter exit value: ");
	gets(ent);

	kexit(ent);
}

int do_wait(){
	int pid, status;
	pid = kwait(&status);
	printf("pid: %d\nstatus: %d\n", pid, status);
}

int do_sleep(){
	//get sleep event
	char ent[64];

	printf("\nEnter sleep value: ");
	gets(ent);
	
	ksleep(atoi(ent));
}

int do_wakeup(){
	//get wakeup event
	char ent[64];

	printf("\nEnter wakeup value: ");
	gets(ent);	
	
	kwakeup(atoi(ent));
}
