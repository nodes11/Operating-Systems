#include "ucode.c"

/******Globals********/
char input[128],
	 input2[128],
	 input3[128],
	 iline[256],
	 *commandArgs[64],
	 *commandArgs2[64],
	 *commandArgs3[64],
	 *head,
	 *tail;
int myargc,
	numPipes,
	numRedirs,
	redirValue,
	redirIndex;
/**********************/

/****************Parses command line*************************/
void parseCommandLine(char *input, char *storeList[]){
	char *s;
	int i = 0;
	
	myargc = 0;
	
	//First we need to seperate our path from our command
	s = strtok(input, " ");
	storeList[i++] = s;
	myargc++;

	//Parse the rest of the command line
	while(s = strtok(0, " ")){
		storeList[i++] = s;
		myargc++;
	}
}

/******************Looks through input for pipe****************/
int findLastPipe(char **tail, char *cmd){
	char *temp = cmd;

	//Move to the end of the string
	while(*temp != 0){
		temp += 1;
	}

	//Now move backwards. Reach the end? return out
	while (*temp != '|'){
		if (temp == cmd)	//No pipes
			return 0;
		temp -= 1;	//Continue to move backwards
	}

	//We have found the last pipe (cat f | cat | more)
	*temp = 0;                   //            ^
	//Go to the space (cat f |(here)grep filename)	
	temp += 1;
	//Go to the first char of the file second command
	temp += 1;
	//Set the tail
	*tail = temp;

	return 1;
}

/********************************************************/
int runCommand(char *cmd){
	char *input3[128], openName[64];
	int i = 0, j = 0, close4What = -1, start = 0;	
	strcpy(input3, cmd);

	start = 0;

	//Parse commandline again
	parseCommandLine(input3, commandArgs2); 

	//Redirection handling
	for (i = 0; i < myargc; i++){
		//If we want to change stdin
		if (strcmp(commandArgs2[i], "<") == 0){
			commandArgs2[i] = 0;
			strcpy(openName, commandArgs2[i+1]); 
			commandArgs2[i + 1] = 0;
			close4What = 0; 
			i = i+1;
		}
		//Stdout for overwriting
		else if (strcmp(commandArgs2[i], ">") == 0){
			commandArgs2[i] = 0;
			strcpy(openName, commandArgs2[i+1]); 
			commandArgs2[i+1] = 0;
			close4What = 1;
			i = i+1;
		}
		//Stdout for appending
		else if (strcmp(commandArgs2[i], ">>") == 0){
			commandArgs2[i] = 0;
			strcpy(openName, commandArgs2[i+1]); 
			commandArgs2[i+1] = 0;
			close4What = 2;
			i = i+1;
		}
		//Otherwise add the command to the command string
		else{
			if (start == 1){
				strcat(iline, " ");
				strcat(iline, commandArgs2[i]);
			}
			else if (start == 0){
				strcpy(iline, commandArgs2[i]);
				start = 1;
			}
		}
	}

	//Depending on what happened above, we might have to close/open some stuff
	if (close4What == 0){
		close(0);
		open(openName, O_RDONLY);
	}
	else if (close4What == 1){
		close(1);
		open(openName, O_WRONLY|O_CREAT);
	}
	else if (close4What == 2){
		close(1);
		open(openName, O_WRONLY|O_CREAT|O_APPEND); 
	}

	//Try running a simple command, or fork a 
	if (doBasicCommand(commandArgs2) == 0){
		strcpy(input3, "/bin/");
		strcat(input3, iline);
		exec(input3);
	}
}

/********************************************************/
int doCommand(char *cmd, int *pipeDescriptor){
	int pd[2], pid;
	char *tail;

	//Child process handling it's stuff.
	//	1. Close reading end of pipe
	//	2. Close STDOUT
	//	3. Replace STDOUT with the pipe out
	if (pipeDescriptor){
		close(pipeDescriptor[0]);
		close(1);
		dup(pipeDescriptor[1]);
	}

	//Is there another pipe? If no, execute the tail
	if (findLastPipe(&tail, cmd) == 0){
		runCommand(cmd);
	}
	else{
		pipe(pd);	//Create our pipe

		pid = fork();	//Fork a child process

		//Parent executing after child dies
		//	1. Close reading end of pipe
		//	2. Close STDOUT
		//	3. Replace STDOUT with the pipe out
		if (pid){
			close(pd[1]);
			close(0);
			dup(pd[0]);
						
			//Back to the parent, run the tail command
			runCommand(tail);
		}
		else{	//Child is going to college. Getting a degreee. They'll die soon enough.
			doCommand(cmd, pd);
		}
	}
}

/************Executes all simple commands*****************/
int doBasicCommand(char *commandArgs[]){
	char *cmd = commandArgs[0],
		 *arg = commandArgs[1];

	if (strcmp(cmd, "exit") == 0){
		exit(0);
	}
	else if (strcmp(cmd, "logout") == 0){
		exit(0);
	}
	else if(strcmp(cmd, "cd") == 0){
		if (arg){
			chdir(arg);
		}
		else{
			chdir("/");
		}
	}
	else if(strcmp(cmd, "pwd") == 0){
		pwd();
	}
	else{
		return 0;
	}
}

/***********Creates a child process******************/
int forkChild(char *cmd){
	int pid, childStatus;

	//Create a child proc
	pid = fork();

	if (pid){	//Parent waiting for child to die
		pid = wait(&childStatus);
	}
	else{	//Child goes to college. Gets a degree. Dies eventually.
		doCommand(cmd, 0);
	}

	return childStatus;
}

/**********************Parses command after input*****************/
void processCommand(char *cmd){
	int i, pipe = 0;

	//Parse command by spaces
	parseCommandLine(cmd, commandArgs);

	//Input2 is still while
	
	//Do we have a pipe? Lets check.
	for (i = 0; i < myargc; i++){
		if (commandArgs[i] == "|"){
			pipe = 1;
		}
	}

	//If we have a pipe, we know we need a child process. So fork one.
	if (pipe == 1){
		forkChild(input2);
	}
	//Otherwise, try a simple command and then forka  child in needed
	else{
		if (doBasicCommand(commandArgs) == 0){
			forkChild(input2);
		}
	}
}

/**************Flushes out input buffers**********************/
void clearBuffers(){
	bzero(input, sizeof(input));
	bzero(input, sizeof(input2));
	bzero(input, sizeof(input3));
	bzero(commandArgs, sizeof(commandArgs));
	bzero(commandArgs2, sizeof(commandArgs2));
	bzero(commandArgs3, sizeof(commandArgs3));
	bzero(iline, sizeof(iline));
}

/**************Gets and splits input from user****************/
int main(int argc, char *argv[]){
	int i, j;

	while(1){
		clearBuffers();

		printf("$ ");
		gets(input);

		strcpy(input2, input); 
		
		//Run the command
		processCommand(input);
	}	
}
