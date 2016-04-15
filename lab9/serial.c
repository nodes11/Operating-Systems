/*******************************************************
 * Tx --> Used for DTE to DCE or Treminal Equipment to Communication equipment
 * Rx --> Used for DCE to DTE
 *
 * Each serial port is represented by an STTY structure
 * 1. -Circular input buff
 * 	  -Semaphore for sync
 *
 * 2. -Circular output buff
 * 	  -Semaphore for sync	
 *
 *
 *
 * RX can be used at any time
 * TX has to be enabled by a "Clear To Send" value
 * ****************************************************/



/********  bgetc()/bputc() by polling *********/
//Put to serial
int bputc(int port, int c)
{
    while ((in_byte(port+LSR) & 0x20) == 0);
    out_byte(port+DATA, c);
}

//Get from serial
int bgetc(int port)
{
    while ((in_byte(port+LSR) & 0x01) == 0);
    return (in_byte(port+DATA) & 0x7F);
}

int enable_irq(u8 irq_nr)
{
   out_byte(0x21, in_byte(0x21) & ~(1 << irq_nr));
}

   
/************ serial ports initialization ***************/
char *p = "\n\rSerial Port Ready\n\r\007";

int sinit()
{
  	int i;  
  	struct stty *t;
	char *q; 

  	/* initialize stty[] and serial ports */
	for (i = 0; i < NR_STTY; i++){
		q = p;

    	printf("sinit : port #%d\n",i);

    	t = &stty[i];

      	/* initialize data structures and pointers */
      	if (i==0)
      	  t->port = 0x3F8;    /* COM1 base address */
      	else
          t->port = 0x2F8;    /* COM2 base address */  

      	t->inchars.value  = 0;  t->inchars.queue = 0;
      	t->outroom.value = BUFLEN; t->outroom.queue = 0;

      	t->inhead = t->intail = 0;
      	t->outhead =t->outtail = 0;

      	t->tx_on = 0;

      	// initialize control chars; NOT used in MTX but show how anyway
      	t->ison = t->echo = 1;   /* is on and echoing */
      	t->erase = '\b';
      	t->kill  = '@';
      	t->intr  = (char)0177;  /* del */
      	t->quit  = (char)034;   /* control-C */
      	t->x_on  = (char)021;   /* control-Q */
      	t->x_off = (char)023;   /* control-S */
      	t->eof   = (char)004;   /* control-D */

    	lock();  // CLI; no interrupts

      	//out_byte(t->port+MCR,  0x09);  /* IRQ4 on, DTR on */ 
      	out_byte(t->port+IER,  0x00);  /* disable serial port interrupts */

      	out_byte(t->port+LCR,  0x80);  /* ready to use 3f9,3f8 as divisor */

      	out_byte(t->port+DIVH, 0x00);
      	out_byte(t->port+DIVL, 12);    /* divisor = 12 ===> 9600 bauds */


      	/******** term 9600 /dev/ttyS0: 8 bits/char, no parity *************/ 
      	out_byte(t->port+LCR, 0x03); 

      	/*******************************************************************
        	Writing to 3fc ModemControl tells modem : DTR, then RTS ==>
        	let modem respond as a DCE.  Here we must let the (crossed)
        	cable tell the TVI terminal that the "DCE" has DSR and CTS.  
        	So we turn the port's DTR and RTS on.
      	********************************************************************/

      	out_byte(t->port+MCR, 0x0B);  /* 1011 ==> IRQ4, RTS, DTR on   */
      	out_byte(t->port+IER, 0x01);  /* Enable Rx interrupt, Tx off */

    	unlock();
    
    	enable_irq(4-i);  // COM1: IRQ4; COM2: IRQ3

    	/* show greeting message */
    	while (*q){
      		bputc(t->port, *q);
      		q++;
    	}
  	}
}  
         
//======================== LOWER HALF ROUTINES ===============================
//Will call shandler upon an iterrupt
int s0handler()
{
	shandler(0);
}
int s1handler()
{
  	shandler(1);
}

//When an interrupt occurs, this is where it's delt with
int shandler(int port)
{  
   struct stty *t;
   int IntID, LineStatus, ModemStatus, intType, c;

   t = &stty[port];            /* IRQ 4 interrupt : COM1 = stty[0] */

   //Used to determine the interrupt case
   IntID     = in_byte(t->port+IIR);       /* read InterruptID Reg */
   //Line and modem errors... Not applicable
   LineStatus= in_byte(t->port+LSR);       /* read LineStatus  Reg */    
   ModemStatus=in_byte(t->port+MSR);       /* read ModemStatus Reg */

   //Handle the interrupt
   intType = IntID & 7;     /* mask out all except the lowest 3 bits */
   switch(intType){
      case 6 : do_errors(t);  break;   /* 110 = errors */
      case 4 : do_rx(t);      break;   /* 100 = rx interrupt */
      case 2 : do_tx(t);      break;   /* 010 = tx interrupt */
      case 0 : do_modem(t);   break;   /* 000 = modem interrupt */
   }
   out_byte(0x20, 0x20);     /* reenable the 8259 controller */ 
}

int do_errors()
{ 
	printf("assume no error\n");
}

int do_modem()
{  
	printf("don't have a modem\n");
}


/* The following show how to enable and disable Tx interrupts */

enable_tx(struct stty *t)
{
	lock();
		out_byte(t->port+IER, 0x03);   /* 0011 ==> both tx and rx on */
		t->tx_on = 1;
	unlock();
}

disable_tx(struct stty *t)
{ 
	lock();
		out_byte(t->port+IER, 0x01);   /* 0001 ==> tx off, rx on */
		t->tx_on = 0;
	unlock();
}

//Copies inout from serial port to terminal
//Usees a circular buffer
int do_rx(struct stty *tty)   /* interrupts already disabled */
{ 
	int c;
	int i = 0;
	c = in_byte(tty->port) & 0x7F;  /* read the ASCII char from port */

	printf("%c", c);	//Print the entered character

	// Write code to put c into inbuf[ ]; notify process of char available;
	
	bputc(tty->port, c);	//Print to the serial

	if (c == '\r'){	//If the user presses enter, show the buf, and then clear it
 		printf("\ntty->inbuf: %s\n", tty->inbuf);

		//Clear the buffer
		while(tty->inbuf[i] != 0){
			tty->inbuf[i] = 0;
			i++;
		}

		//Set the head and tail back to the start
		tty->inhead = tty->intail = 0;
		return;
	}

	//If the user has typed to much (overflow)
  	if (tty->inchars.value >=BUFLEN){
		out_byte(tty, BEEP);
		return;
  	}

	//Adds newline for cntrl-c
  	if (c == 0x3){
  		c = '\n';
  	}

	//Set the buffer[i] value
  	tty->inbuf[tty->inhead++] = c;
  	tty->inhead %= BUFLEN;

	//Wake up the semaphore
  	V(&tty->inchars);
}      
     
int sgetc(struct stty *tty)
{ 
  	//write Code to get a char from inbuf[ ]
	char c;
	
	P(&tty->inchars); // wait if no input char yet
	
	lock(); // disable interrupts

		c = tty->inbuf[tty->intail++];
		tty->intail %= BUFLEN;
	
	unlock(); // enable interrupts
	
	return c;
}

//Called from sin()
int sgetline(struct stty *tty, char *line)
{  
   // write code to input a line from tty's inbuf[ ] 
   int i = 0;
   char c;

	while(line[i] != '\0'){
		c = sgetc(tty);
		line[i++] = c;
	}

    return strlen(line);
}


/*********************************************************************/

//Prints text from QEMU to serial port
int do_tx(struct stty *tty)
{
	int c;

	printf("tx interrupt ");
	
	//If we have read all the chars in the out buf... aka head == tail
	//We want to disable the tx_interrupt
	if (tty->outhead == tty->outtail){
		disable_tx(tty);
    	return;
  	}

	//Get the next char from the buf and display
  	c = tty->outbuf[tty->outtail++];
  	tty->outtail %= BUFLEN;
  	out_byte(tty->port, c); //end of interrupt

	//Wakeup putc or unblock it
  	V(&tty->outroom);
}

int sputc(struct stty *tty, int c)
{
  	// write code to put c into tty's outbuf[ ]
  	
	P(&tty->outroom); // wait for space in outbuf[]

	lock(); // disalble interrupts
		
		tty->outbuf[tty->outhead++] = c;
		tty->outhead %= BUFLEN;
	 
		if (!tty->tx_on)
			enable_tx(tty); // trun on tty’s tx inte

	unlock();	//Reenable interrupts
}

//Called from sout()
int sputline(struct stty *tty, char *line)
{
	// write code to output a line to tty
	int i = 0;

	//Print to the serial port
	while (line[i] != 0){
		sputc(tty, line[i++]);
  	}

	//Make sure we are actully putting the right thing (DEBUG)
	printf("Putting to terminal: %s\n", line);
}
