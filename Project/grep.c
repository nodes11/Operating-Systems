#include "ucode.c"

int main(int argc, int *argv[]){
	char *find, *filename,
		 buf[1024], line[1024], currPort[128];
	int readIn, readThisMuch, totalRead;
	int i, j, k, fd = STDIN, con = 0, redir0 = 0, redir1 = 0, numBytes = 0;

	//From the book
	STAT st_tty, st0, st1, infileStat;
	gettty(currPort);
	con = stat(currPort, &st_tty);
	fstat(0, &st0);
	fstat(1, &st1);

	//Check for redirection on stdin
	if (st_tty.st_dev != st0.st_dev || st_tty.st_ino != st0.st_ino) //0 has been redirected
		redir0 = 1;
	//Check for redirection on stdout
	if (st_tty.st_dev != st1.st_dev || st_tty.st_ino != st1.st_ino)	//1 has been redirected
		redir1 = 1;

	//Did they give the right number of arguments?
	if (argc < 2 || argc > 3){
		printf("error: invalid number of arguments\n");
	}
	else if (argc == 2){
		find = argv[1];
		//Case with no file
		
		if (redir0 == 1){						//STDIN is redirected
			/********************************************************
			 * Tried writing with the readin loop below in argc == 3.
			 * Wasn't reading in lines correctly. Realize that getline
			 * was implemented in the ucode.c.
			 *
			 * This works 90% of the time
			 * ******************************************************/
			//Read from redirected input and display to screen/stdout
			while(getline(line)){
				if (strstr(line, find)){
					//write(1, line, strlen(line));
					printf("%s", line);
					write(2, "\r", 1);
				}
			}
		}
		else{							//No STDIN redirection
			/*******************************************************
			 * Get a single line of input from the user. Print it to
			 * screen if the pattern matches.
			 *
			 * WORKS
			 * ****************************************************/
			//Deals with case "grep f" and reads input from keyboard
			while(gets(line)){
				if (strstr(line, find)){
					printf("%s\n", line);
				}
			}
		}
	}
	else if (argc == 3){				//User gives a file name argument
		find = argv[1];
		filename = argv[2];
		fd = open(filename, O_RDONLY);
		stat(filename, &infileStat);

		/*******************************************
		 * Given a file, read up in
		 *
		 *
		 * ***************************************/
		if(fd >= 0){
			printf("%s opened successfully\n", filename);

			//Figure out how much we want to read in from the file
			if (infileStat.st_size < 1024){
				readThisMuch = infileStat.st_size;
			}
			else{
				readThisMuch = 1024;
			}

			j = 0;
			//READ!!!!!!!!
			while((readIn = read(fd, buf, readThisMuch)) > 0){
				i = 0;

				//Read from the buffer until we read the end
				while(buf[i]){
					//Read in until the newline
					if (buf[i] == '\n' || buf[i] == '\r'){
						line[j] = 0;
						//Look for thie pattern
						if (strstr(line, find)){
							k = 0;
							printf("%s\n", line);
						}
			

						j = 0;	
						//Clear line so we can read more in from buf
						while(line[j] != 0){
							line[j++] = 0;
						}
						j = 0;
					}
					//No newline? Append to to line from buf
					else{
						line[j] = buf[i];
						j++;
					}
					i++;
				}	

				//Calculate how much more to read from the file
				if (infileStat.st_size - totalRead < 1024){
					readThisMuch = infileStat.st_size - totalRead;
				}
				else{
					readThisMuch = 1024;
				}

				//clear our buffers
				bzero(line, sizeof(line));
				bzero(buf, sizeof(buf));
			}
		}		
	}

	exit(0);	
}
