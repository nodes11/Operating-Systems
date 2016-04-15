// ucode.c file

char *cmd[]={"getpid", "ps", "chname", "kfork", "switch", "uwait", "exit", "getc", "putc", "fork", "exec", "pipe", "pfd", "read", "write", "close", "show", "sleep", 0};

#define LEN 64

int show_menu()
{
   printf("\n***************************************** Menu **********************\n");
   printf("* getpid | ps | chname | kfork | switch | uwait | exit | getc | putc *\n");
   printf("* fork | exec | pipe | pfd | read | write | close | show | sleep     *\n");
   printf("*********************************************************************\n");
}

int find_cmd(char *name)
{
	// return command index
	int i = 0;


	//Compare the entered command witht he command list until a valid comman is found
	for (i = 0; i < 18; i++){
		if (strcmp(name, cmd[i]) == 0){
			return i;
		}
	}
	return -1;
}

int strcmp(char *s1, char *s2){
	int i = 0;

	if (*s1 == *s2){
		//Loop until we find a difference
		while (*s1 && *s2 && *++s1 == *++s2){printf("");};
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

/************************************************************/
//In order for the keyboard driver to go back to UMODE and
//execute a command I typed in I had to rewrite the gets and 
//command. It is similar to the one from Lab #1. KC's gets was
//hanging up and wasn't recognizing to end the interrupt process
//when I pressed enter.
int ugetc(){
	char c = syscall(7,0,0);

	while (c == 0){
		c = syscall(7, 0, 0);
	}

	return c;
}

int uputc(){
	return syscall(8, ugetc(),0);
}

int ugets(char str[]){
	//Checks for '\n' rather than '\r'
	while((*str = ugetc()) != '\n'){
		*str++;
	}

	//Remove the extra character
	*(str-1) = 0;
	
	return; 
}


/************************************************************/

int fork(){
	return syscall(9, 0, 0);
}

int exec(char *s){
	return syscall(10, s, 0);
}

int ufork() // user fork command
{ 
	int child = fork();

	(child) ? printf("parent ")
		   : printf("child ");
	printf("%d return form fork, child_pid=%d\n", getpid(), child);
}

int uexec() // user exec command
{
	int r; 
	char filename[64];
	
	printf("enter exec filename: ");
	gets(filename);

	printf("%s", filename);

	r = exec(filename);
	printf(" exec failed\n");
}

int pd[2];

int pipe()
{
   printf("pipe syscall\n");
   syscall(11, pd, 0);
   printf("proc %d created a pipe with fd = %d %d\n", 
           getpid(), pd[0], pd[1]);
}

int pfd()
{
	return syscall(12,0,0);
}

int read_pipe()
{
  char buf[1024]; 
  int fd, n, nbytes;
  pfd();

  printf("Read from which fd: ");
  fd = atoi(gets());

  printf("Read how many bytes: ");
  nbytes = atoi(gets());

  n = syscall(13, fd, buf, nbytes);

  if (n>=0){
     printf("proc %d back to Umode, read %d bytes from pipe : ",
             getpid(), n);
     buf[n]=0;
     printf("%s\n", buf);
  }
  else
    printf("read pipe failed\n");
}

int write_pipe()
{
  char buf[1024]; 
  int fd, n, nbytes;
  pfd();

  printf("Write to which fd: ");
  fd = atoi(gets());

  printf("Write what string: ");
  gets(buf);  

  nbytes = strlen(buf);
            
  printf("fd=%d nbytes=%d : %s\n", fd, nbytes, buf);

  n = syscall(14,fd,buf,nbytes);

  if (n>=0){
     printf("\nproc %d back to Umode, wrote %d bytes to pipe\n", getpid(),n);
  }
  else
    printf("write pipe failed\n");
}

int close_pipe(){
	int pNum;
	pfd();
	printf("Close fd #: ");
	pNum = atoi(gets());
	return syscall(15, pNum, 0);
}

int show_pipe(){
	int pNum;
	printf("PIPE #: ");
	pNum = atoi(gets());

	return syscall(16, pNum, 0);
}


//NEW UCODE CALL FOR SLEEP
int usleep(){
	int s;
	printf("Sleep for: ");
	s = atoi(gets());
	syscall(17, s, 0);
}
