#include "ucode.c"

int main(int argc, int *argv[]){
	char *filename, buf, currPort[128], conInput;
	int i = 0, fd = STDIN, readIn, con = 0, redir0 = 0, redir1 = 0, numLines = 0;

	//From the book
	STAT st_tty, st0, st1;
	gettty(currPort);
	con = open(currPort, 0);
	stat(currPort, &st_tty);
	fstat(0, &st0);
	fstat(1, &st1);

	//Is stdin redirected?
	if (st_tty.st_dev != st0.st_dev || st_tty.st_ino != st0.st_ino) //0 has been redirected
		redir0 = 1;

	//Is stdout redirected?
	if (st_tty.st_dev != st1.st_dev || st_tty.st_ino != st1.st_ino)	//1 has been redirected
		redir1 = 1;

	//Do we have a file? Open it.
	if (argc == 2){
		//Open up the file
		filename = argv[1];
		fd = open(filename, O_RDONLY);
	}

	if (fd <= 0){
		if (redir0 == 1){
			readIn = read(0, &buf, 1);	//Read in our first byte
		
			while(readIn > 0){
				//We know the screen is about 25 lines tall. So we want to print 25 lines
				//initially, and then wait for a keyboard interrupt to print more.
				while(numLines < 25){
					if (buf == '\0')	//Have we reached the end of the file? break.
						break;

					if (buf == '\n'){		//Have we reached a newline? Print a carraige return and newline. 
						write(2, "\r", 1);
						write(1, "\n", 1);
						numLines++;			//Start of new line
					}
					else{					//Otherwise write char to stdout
						write(1, &buf, 1);
					}

					readIn = read(0, &buf, 1);	//Readin the next char 

					if (readIn == 0){	//Did we just read a null character? If so, break out.
						numLines = 26;
						break;
					}
				}

				if (numLines == 25){	//We have printed 25 lines
					read(con, &conInput, 1);	//Wait for keyboard interrupt to print more
				
					if (conInput == 32){	//If we get a space, print and entire screen
						numLines = 0;
					}
					else{					//Any other key, print a single line
						numLines--;
					}
				}
			}
		}
	}
	else{
		/************************************************************
		 * Dealing with a case where we have a file.                *
		 * **********************************************************/
			readIn = read(fd, &buf, 1);	//Read in our first byte
			while(readIn > 0){
				//We know the screen is about 25 lines tall. So we want to print 25 lines
				//initially, and then wait for a keyboard interrupt to print more.
				while(numLines < 25){
					if (buf == '\0')	//Have we reached the end of the file? break.
						break;

					if (buf == '\n'){		//Have we reached a newline? Print a carraige return and newline. 
						write(2, "\r", 1);
						write(1, "\n", 1);
						numLines++;			//Start of new line
					}
					else{					//Otherwise write char to stdout
						write(1, &buf, 1);
					}

					readIn = read(fd, &buf, 1);	//Readin the next char 

					if (readIn == 0){	//Did we just read a null character? If so, break out.
						numLines = 26;
						break;
					}
				}

				if (numLines == 25){	//We have printed 25 lines
					read(con, &conInput, 1);	//Wait for keyboard interrupt to print more
					
					if (conInput == 32){	//If we get a space, print and entire screen
						numLines = 0;
					}
					else{					//Any other key, print a single line
						numLines--;
					}
				}
			}
	}

	close(fd);
	exit(0);		
}
