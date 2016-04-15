// ucode.c file

char *cmd[]={"getpid", "ps", "chname", "kfork", "switch", "wait", "exit", "getc", "putc", 0};

#define LEN 64

int show_menu()
{
   printf("********************************* Menu ******************************\n");
   printf("* getpid | ps | chname | kfork | switch | wait | exit | getc | putc *\n");
   printf("*********************************************************************\n");
}

int find_cmd(char *name)
{
	// return command index
	int i = 0;


	//Compare the entered command witht he command list until a valid comman is found
	for (i = 0; i < 9; i++){
		
		if (strcmp(name, cmd[i]) == 0){
			return i;
		}
	}
	return -1;
}

int strcmp(char *s1, char *s2){
	int i = 0;

	//check to make sure s1 and s2 exist
	if (*s1 == *s2){
		//Loop until we find a difference
		while (*s1 && *s2 && *s1++ == *s2++){printf("");};
	}
	//Works just like normal strcmp. Will return the difference in length if different.
	return *s1 - *s2;
}

int getpid()
{
   return syscall(0,0,0);
}

int ps()
{
   return syscall(1, 0, 0);
}

int chname()
{
    char s[32];	//32 bytes (Not 64 as in the PROC structure)
    printf("input new name : ");
    gets(s);
    return syscall(2, s, 0);
}

int kfork()
{   
  int child, pid;
  pid = getpid();
  printf("proc %d enter kernel to kfork a child\n", pid); 
  child = syscall(3, 0, 0);
  printf("proc %d kforked a child %d\n", pid, child);
}    

int kswitch()
{
    return syscall(4,0,0);
}

int wait()
{
    int child, exitValue;
    printf("proc %d enter Kernel to wait for a child to die\n", getpid());
    child = syscall(5, &exitValue, 0);
    printf("proc %d back from wait, dead child=%d", getpid(), child);
    if (child>=0)
        printf("exitValue=%d", exitValue);
    printf("\n"); 
} 

int geti()
{
  	// This is a 16 bit OS (MAX) so we need 16 items in our array
	char returnValue[16];

	// Return an input integer
	return atoi(gets(returnValue));
}

int exit()
{
   int exitValue;
   printf("enter an exitValue: ");
   exitValue = geti();
   printf("exitvalue=%d\n", exitValue);
   printf("enter kernel to die with exitValue=%d\n", exitValue);
   _exit(exitValue);
}

int _exit(int exitValue)
{
  return syscall(6,exitValue,0);
}

int invalid(char *name)
{
    printf("Invalid command : %s\n", name);
}

int ugetc(){
	return syscall(7, 0, 0);
}

int uputc(){
	return syscall(8, ugetc(),0);
}
