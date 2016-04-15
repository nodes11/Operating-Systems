/******************** timer.c file *************************************/
#define LATCH_COUNT     0x00	   /* cc00xxxx, c = channel, x = any */
#define SQUARE_WAVE     0x36	   /* ccaammmb, a = access, m = mode, b = BCD */

#define TIMER_FREQ   1193182L	   /* clock frequency for timer in PC and AT */
#define TIMER_COUNT  TIMER_FREQ/60 /* initial value for counter*/

#define TIMER0       0x40
#define TIMER_MODE   0x43
#define TIMER_IRQ       0

int tick=0,
   	seconds = 0,
   	minutes = 0,
   	hours = 0;

//int row, column;

/*int enable_irq(u16 irq_nr)
{
  lock();
    out_byte(0x21, in_byte(0x21) & ~(1 << irq_nr));

}*/

int timer_init()
{
  /* Initialize channel 0 of the 8253A timer to e.g. 60 Hz. */

  printf("timer init\n");
  tick = 0; 
  out_byte(TIMER_MODE, SQUARE_WAVE);	// set timer to run continuously
  out_byte(TIMER0, TIMER_COUNT);	// timer count low byte
  out_byte(TIMER0, TIMER_COUNT >> 8);	// timer count high byte 
  enable_irq(TIMER_IRQ);	//Enable the timer interupt 
}

/*===================================================================*
 *		    timer interrupt handler       		     *
 *===================================================================*/
int thandler()
{
  PROC *p;	//Use to go through the sleepList
		
  tick++; 	
  tick %= 60;

  row = 24;	//Use for placement of the time (FULL RES: 24x80)
  column = 70;

  p = sleepList;	//Point to the front of the sleepList

  if (tick == 0){                      // at each second
	  seconds++;

	  if (seconds % 60 == 0){
		  minutes++;
	  	  seconds = 0;
	  }
	  if (minutes != 0 && minutes % 60 == 0 && seconds % 60 == 0){
		  hours++;
		  minutes = 0;
		  if (hours == 24){
			hours = 0;
		  }
	  }


	  //Print the the time using the putc function in the vid.c file
	  vidputc('0' + hours/10);
	  vidputc('0' + hours%10);
	  vidputc(':');
	  vidputc('0' + minutes/10);
	  vidputc('0' + minutes%10);
	  vidputc(':');
	  vidputc('0' + seconds/10);
	  vidputc('0' + seconds%10);
  	  row = 24; column = 70;


	  //Check to wakeup sleeping processes
	  while (p){
		if (p->time == 0){
			kwakeup(p->event);
		}
		else {
			p->time--;
		}

		p = p->next;
	  }

	  if (running->inkmode == 1){ 
		  running->time--;
	  }
  }
 
  //END OF THE TIMER INTERPUT PROCESS SO CALL OUTBYTE 
  out_byte(0x20, 0x20);                // tell 8259 PIC EOI
  
  //Make sure there aren't any extra times on the screen
  clearTime();
}
