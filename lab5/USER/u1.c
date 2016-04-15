#include "ucode.c"
int color;
main()
{ 
  char name[64]; 
  int pid, cmd;

	printf("HI WELCOME TO U1.c\n");

  while(1){
	printf("");
	pid = getpid();
    color = 0x0C;
       
    //printf("----------------------------------------------\n");
    printf("I am proc %d in U mode: running segment=%x\n",getpid(), getcs());
    show_menu();
    printf("UMODE: Enter your command:");
    gets(name); 

    if (name[0]==0) 
        continue;

    cmd = find_cmd(name);
    switch(cmd){
			case 0 : printf("PID: %d\n", getpid());   break;
            case 1 : ps();       break;
            case 2 : chname();   break;
            case 3 : kfork();    break;
            case 4 : kswitch();  break;
            case 5 : wait();     break;
            case 6 : exit();     break;
			case 7 : ugetc();	 break;
			case 8 : uputc();	 break;
			case 9 : ufork();	 break;
			case 10 : uexec();	 break;

            default: invalid(name); break;
    }
  }
}



