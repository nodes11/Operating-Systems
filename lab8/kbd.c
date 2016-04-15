/**********************************************************************
                         kbd.c file
***********************************************************************/
#define KEYBD	         0x60	/* I/O port for keyboard data */
#define PORT_B           0x61   /* port_B of 8255 */
#define KBIT	         0x80	/* bit used to ack characters to keyboard */

#define NSCAN             64	/* Number of scan codes */
#define KBLEN             64    // input buffer size

#define BELL 0x07
#define F1 0x3B // scan code of function keys
#define F2 0x3C
#define CAPSLOCK 0x3A // scan code of special keys
#define LSHIFT 0x2A
#define RSHIFT 0x36
#define CONTROL 0x1D
#define ALT 0x38
#define DEL 0x0E
#define ENTER 0x1C

#include "keymap.c"

extern int color;

typedef struct kbd{           // data struct representing the keyboard
    char buf[KBLEN];          // input buffer
    int  head, tail;          // CIRCULAR buffer
    SEMAPHORE  data;                // number of keys in bif[ ] 
    SEMAPHORE lock;
}KBD;

KBD kbd;

int alt, capslock, esc, shifted, control, arrowKey, enter, delete; // state variables

//Initialize the keyboard, called on every gets() call
int kbd_init()
{
	int i = 0;
	printf("kbinit()\n");

	//Clear the kbd buffer
	while (i < KBLEN){
		kbd.buf[i] = 0;
		i++;
	}

	//Set the head and tail the start
	kbd.head = kbd.tail = 0;

	//Initialize the semaphore queue and value
	kbd.data.value = kbd.data.queue = 0;

	printf("kbinit done\n"); 
}

/******************************************************************************
 kbhandler() is the kbd interrupt handler. The KBD generates 2 interrupts
 for each key typed; one when the key is pressed and another one when the
 key is released. Each key generates a scan code. The scan code of key
 release is 0x80 + the scan code of key pressed. When the kbd interrupts,the
 scan code is in the data port (0x60) of the KBD interface. Read the scan code
 from the data port. Then ack the key input by strobing its PORT_B at 0x61.
*****************************************************************************/


int kbhandler()
{
	int scode, value, c, i;
	KBD *kp = &kbd;

	//(1). get scan code; ACK to port;
	// Fetch scan code from the keyboard hardware and acknowledge it.
	scode = in_byte(KEYBD);	 // get the scan code of the key 
	value = in_byte(PORT_B);	 // strobe the keyboard to ack the char 

	//Acknowledging that a value has been given
	out_byte(PORT_B, value | KBIT);// first, strobe the bit high 
	out_byte(PORT_B, value);	 // then strobe it low 


	if (scode & 0x80){// key release: ONLY catch shift,control,alt
		scode &= 0x7F; // mask out bit 7
		
		if (scode == LSHIFT || scode == RSHIFT){
			shifted = 0; // released the shift key
		}
		if (scode == CONTROL)
			control = 0; // released the Control key
		if (scode == ALT)
			alt = 0; // released the ALT key
		goto out;
	}




	if (scode == LSHIFT || scode == RSHIFT){
		shifted = 1; // set shifted flag
		goto out;
	}




	if (scode == ALT){
		alt = 1;
		goto out;
	}

	if (scode == CONTROL){
		control = 1;
		goto out;
	}




	if (scode == 0x3A){
		capslock = 1 - capslock; // capslock key acts like a toggle
		goto out;
	}

	/************* Catch and handle F keys for debugging *************/
	if (scode == F1){	//Call ps to print all processes 
		kps(); goto out;
	}

	if (scode == F2){	//Fork a process
		fork(); 
		goto out;
	}




	//Get the proper character
	//Check to see if it's shifted or not
	if (shifted == 1){
		c = shift[scode];
	}
	else{
		c = unshift[scode];
	}

	// Convert all to upper case if capslock is on
	if (capslock){
		if (c >= 'A' && c <= 'Z')
			c += 'a' - 'A';
		else if (c >= 'a' && c <= 'z')
			c -= 'a' - 'A';
	}

	//Show the character on the screen
	putc(c);

	//Store the character in the KBD buffer
	kbd.buf[kbd.head++] = c;
	//Set head to the right spot
	kbd.head %= KBLEN;
	V(&kbd.data);

	//If the user presses ENTER we want to go back to UMODE
	if (scode == ENTER){
		enter = 1;					//Used in OUT
		kbd.buf[kbd.head] = '\n';	//Set the last character to '\n'
		goto out;					//Go to OUT to end the interrupt
	}




	//Called at the end of the interrupt
	out:
		if (enter == 1){			//If we have an enter:
			putc('\n');				//Print newline
			putc('\r');				//Move cursor back
			enter = 0;				//Reset global
		}

		//End the interrupt
		out_byte(0x20, 0x20);
}

/********************** upper half rotuine ***********************/ 
int kbd_getc()
{
	u8 c;

	if (running->fground==0)
		P(&kbd.lock); // only foreground proc can getc() from KBD

	P(&kbd.data);
	lock();

	c = kbd.buf[kbd.tail];
	kbd.buf[kbd.tail] = 0;
 
	kbd.tail++;
	kbd.tail %= KBLEN;

	unlock();
 	return c;
}


/******************************************************************
WE DON"T WANT TO SLEEP OR BLOCK BECAUSE:

1. We don't want to sleep or block beacuse we don't want to sleep or 
block and interrupt.
******************************************************************/

//CALLED in getc and waits for a character to be entered by the user
P(SEMAPHORE *s)
{ 
	int_off();	
	if (--s->value < 0)
		wait(s->queue);
	int_on();
}

//Called in the handler once a character was entered
V(SEMAPHORE *s)
{ 
	int_off();
	if (++s->value <= 0)
		signal(s->queue);
	int_on();
}

