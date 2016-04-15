#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procSize = sizeof(PROC);
int nproc = 0;

int body();
char *pname[]={"Sun", "Mercury", "Venus", "Earth",  "Mars", "Jupiter", 
               "Saturn", "Uranus", "Neptune" };

/**************************************************
  bio.o, queue.o loader.o are in mtxlib
**************************************************/
#include "queue.c"
#include "wait.c"             // YOUR wait.c   file
#include "kernel.c"           // YOUR kernel.c file
#include "int.c"              // YOUR int.c    file
int color;

/************************************************************************
 * Each KMODE process has:
 *	- kstack - gives context of execution
 *	- When a process starts running again, it starts where it left off
 * Each UMODE process has:
 * 	- Each process has its own umode segment (0x2000 --> 0x9000)
 * 	- Each segment contains Ucode, Udata, and the Ustack
 *
 *
 * When executing a UMODE process the CPU registers CS, DS, and PS point to
 * the proper memory area. When going back to kmode they point at 0x1000
 * or the kernel memory area.
 ************************************************************************/


int body(){
	char c;

    
    goUmode();

    printf("\nproc %d starts from body()\n", running->pid);

    while(1) {
		color = running->pid + 1;	//Change the color depending on the running process

		printf("\n---------------------------------------------------------------\n\n");
		
		printf("freeList: ");// optional: show the freeList
		printList(freeList,0); 
		putc('\n');

		printf("\nreadyQueue: "); // show the readyQueue
		printList(readyQueue,0);
		putc('\n');

		printf("\nsleepList: "); 
		printList(sleepList,1);
		putc('\n');
		
		printf("\nproc %d running: parent=%d\n",running->pid, running->ppid);
		printf("Enter a char [s|f|q|z|a|w|u]: ");
		c = getc(); 
		printf("%c\n", c);

        switch(c){
            case 'f' : do_kfork(); break;
            case 's' : do_tswitch(); break;
			case 'q' : do_exit(); break;
 			case 'z' : do_sleep(); break;
 			case 'a' : do_wakeup(); break;
			case 'w' : do_wait(); break;
			case 'u' : do_umode(); break;
			default : printf("Invalid Command\n"); break;
        }
    }
}

int init(){
    PROC *p; int i;
    color = 0x0C;

    for (i=0; i<NPROC; i++){   // initialize all procs
        p = &proc[i];
        p->pid = i;
        p->status = FREE;
        p->priority = 0;  
        strcpy(proc[i].name, pname[i]);
        p->next = &proc[i+1];
    }

    freeList = &proc[0];      // all procs are in freeList
    proc[NPROC-1].next = 0;
    readyQueue = sleepList = 0;

    /**** create P0 as running ******/
    p = get_proc();
    p->status = RUNNING;
    p->ppid   = 0;
    p->parent = p;
    running = p;
    nproc = 1;
} 

int scheduler()
{
    if (running->status == RUNNING){
		running->status = READY;
    	enqueue(&readyQueue, running);
	}
    running = dequeue(&readyQueue);
	running->status = RUNNING;
    color = running->pid + 0x0A;
}

int int80h();

int set_vector(u16 vector, u16 handler){
     // put_word(word, segment, offset)
    put_word(handler, 0, vector<<2);
	put_word(0x1000,  0, (vector<<2) + 2);
}

main()
{
    printf("MTX starts in main()\n\n");

	printf("Initializing...\n");
	init();      				// initialize and create P0 as running
	printf("Initialization complete...\n\n");

    	set_vector(80, int80h);		//Set vector 80 as int80h (in assembly)

	printf("Forking P1...\n");
   	kfork("bin/u1");			// P0 kfork() P1
	printf("Forking compelete...\n\n");
   
	while(1){
		printf("P0 running\n");

		while(!readyQueue);
      
		printf("P0 switch process\n");
      
		tswitch();	// P0 switch to run P1
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

int do_umode(){
	goUmode();
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
