#include "ucode.c"

int main(int argc, int *argv[]){
	char *filename, buf, line[1024], currPort[128];
	int i, fd = STDIN, readIn, con = 0, redir0 = 0, redir1 = 0, numBytes = 0;

	//From the book
	STAT st_tty, st0, st1;
	gettty(currPort);
	con = stat(currPort, &st_tty);
	fstat(0, &st0);
	fstat(1, &st1);

	//Check for redirection of stdin
	if (st_tty.st_dev != st0.st_dev || st_tty.st_ino != st0.st_ino) //0 has been redirected
		redir0 = 1;

	//Check for redirection of stdout
	if (st_tty.st_dev != st1.st_dev || st_tty.st_ino != st1.st_ino)	//1 has been redirected
		redir1 = 1;

	//Do we have a file? Open it for reading
	if (argc == 2){
		//Open up the file
		filename = argv[1];
		fd = open(filename, O_RDONLY);
	}


	/***********************************
	 * DEALING WITH "CAT F" BASIC CASE *
	 * *********************************/
	if (fd > 0){
		while(readIn = read(fd, &buf, 1)){	//Start reading a byte at a time
			if (buf == '\n'){	//We have newline. Write a carriage return and newline
				write(2, "\r", 1);
				write(1, &buf, 1);
			}
			else{	//Just write the character
				write(1, &buf, 1);
			}
		}
	}
	/***********************************
	 * DEALING WITH REDIRECTION CASES  *
	 * *********************************/
	else{
		while(readIn = read(fd, &buf, 1)){
			//IN UNIX '\n' is the end of line character. Thus why we need to change it.
			if (buf == '\r')
				buf = '\n';
			
			/***************************************************
			* Dealing with cases that have no STDIN redirection*
			****************************************************/	
			if (redir0 == 0){
				//Add the latest char to the buffer
				line[numBytes++] = buf;

				//Do we have a newline? Write a cr and newline to the console (STDERR)
				if (buf == '\n')
					write(2, "\r\n", 2);
				else	//Otherwise write the latest char to the console (STDERR)
					write(2, &buf, 1);

				//Once we get a newline from the user, we want to write the enitre line to STDOUT
				if (buf == '\n'){
					//Write the line to stdout
					write(1, line, numBytes);

					//In the case where we just have "cat" with no redirection on STDOUT
					//write a cr to the console
					if (redir1 == 0){
						write(2, "\r", 1);
					}
					
					//Set the line buffer position back to the start
					numBytes = 0;
				}
			}
			/************************************************
			* Dealing with cases that have STDIN redirection*
			*************************************************/
			else{
				//If there is no redirection on STDOUT
				if (redir1 == 0){
					//If there is a new line, write a cr to the console
					if (buf == '\n')
						write(2, "\r", 1);
				}
				write(1, &buf,1);	//Write the latest char to the console as well
			}
		}
	}

	close(fd);
	exit(0);		
}

