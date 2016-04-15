//CALLED in getc and waits for a character to be entered by the user
int P(struct semaphore *s)
{ 
	int_off();	
	if (--s->value < 0)
		wait(s->queue);
	int_on();
}

//Called in the handler once a character was entered
int V(struct semaphore *s)
{ 
	int_off();
	if (++s->value <= 0)
		signal(s->queue);
	int_on();
}
