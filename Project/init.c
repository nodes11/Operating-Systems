int pid, child, status, console[3];
int STDIN = 0, STDOUT = 1;

#include "ucode.c"

int main(){
	int in, out, i;

	in = open("/dev/tty0", O_RDONLY);
	out = open("/dev/tty0", O_WRONLY);

	printf("INIT: forks a new console login\n");

	//We want to fork three processes
	//Each will handle one of three (console, serial port 0, sp0)
	for (i = 0; i < 3; i++){
		console[i] = fork();
		
		//If the console isn't occupied. Executed login on it.
		if (!console[i]){
			if (i == 0){
				exec("login /dev/ttyS0");	
			}
			else if (i == 1){
				exec("login /dev/ttyS1");
			}
			else if (i == 2){
				exec("login /dev/tty0");
			}
		}

	}
	
	//Time to wait for some kids to die
	parent();
}

int parent()
{
	int pid, status, forkAndLogin;

	//Waiting loop
	while(1){
		pid = wait(&status);
		
		//SOMEONE DIED! QUICK! WHO WAS IT?
		if (console[0] == pid){
			forkAndLogin = 1;
		}
		else if (console[1] == pid){
			forkAndLogin = 2;
		}
		else if (console[2] == pid){
			forkAndLogin = 3;
		}

		//If kid died. MAKE A NEW ONE!!!
		if (forkAndLogin > 0){
			console[forkAndLogin - 1] = fork();
			if (console[forkAndLogin - 1])
				continue;
			else{
				if (forkAndLogin - 1 == 0){
					exec("login /dev/ttyS0");	
				}
				else if (forkAndLogin - 1 == 1){
					exec("login /dev/ttyS1");
				}
				else if (forkAndLogin - 1 == 2){
					exec("login /dev/tty0");
				}
			}
		}

		//Hurry up and wait
		forkAndLogin = 0;
	}
}
