//#include "loader.c"

PROC *kfork(char *filename) // create a child process, begin from body()
{
	int i, pid;
	u16 segment;
    PROC *p = get_proc();	//Get a PROC from the freelist

    if (p == 0) {	//Make sure there was a PROC to get
        printf("no more PROC, kfork() failed\n");
        return 0;
    }

    p->status = READY;	//Set it's status to ready
    p->priority = 1; // priority = 1 for all proc except P0
    p->ppid = running->pid; // parent = running
	p->parent = running;	//set the parent proc pointer
	
    /* initialize new proc's kstack[ ] */
   	for (i=1; i<10; i++){ // saved CPU registers
        p->kstack[SSIZE-i] = 0; // Set all registers to null
	}
    
    p->kstack[SSIZE-1] = (int)body;
    p->ksp = &p->kstack[SSIZE-9]; // proc saved sp


/*******************************************************************************
      new PROC
        ------
        |uss | = new PROC's SEGMENT value
        |usp | = -24                                    
        --|---                                    Assume P1 in segment=0x2000:
          |                                              0x30000  
          |                                              HIGH END of segment
  (LOW) | |   ----- by SAVE in int80h ----- | by INT 80  |
  --------|-------------------------------------------------------------
        |uDS|uES| di| si| bp| dx| cx| bx| ax|uPC|uCS|flag|NEXT segment
  ----------------------------------------------------------------------
         -12 -11 -10  -9  -8  -7  -6  -5  -4  -3  -2  -1 |


LOW									HIGH
___________________________________________________________________________
CODE        |DATA          |BSS          |STACK                            |
____________|______________|_____________|_________________________________|
^                                        ^                                 ^
0x1000                                  0x1000-24                          0x2000 

********************************************************************************/

	//Checks for the filename argument... In this case u1 located in /bin/...
	if (filename){
		p->uss = ((p->pid+1)*0x1000);	//Set the UMODE segment register anywhere from 0x2000 to 0x9000
		
		load(filename, p->uss);	//Will load the given executable binary file to the processes stack (sets value to image)

		p->usp = -24;	

		for (i = 3; i < 11; i++){
			put_word(0,p->uss,i*-2);	//Fill the low end (int80) registers with 0's. Make sure offset is correct (2 bytes!)!
		}
		
		//SET ALL THE STACK SEGMENTS!!!
		put_word(p->uss, p->uss, -24);	//uDS | starting from the top of the stack on going back (uDS is at the low end)
		put_word(p->uss, p->uss, -22);	//uES same ^
		put_word(p->uss, p->uss, -4);		//uCS same ^^
		put_word(0x200,p->uss, -2);		//flag
		
	}
	else{
		p->uss = (0*0x1000);	//Set UMODE segment register to null
	}

	enqueue(&readyQueue, p); // enter p into readyQueue by priority
	nproc++;	//We have created a new child process, incremnet proc count

	return p; // return child PROC pointer
}
