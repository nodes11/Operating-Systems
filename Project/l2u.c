#include "ucode.c"

int main(int argc, int *argv[]){
	char buf[1024];
	int i = 0, infile, outfile, readIn;

	//In a case with some sort of redirection
	if(argc == 1){
		//Read a line form the buffer
		while (getline(buf)){
			i = 0;
			//Iterate through the line
			while(buf[i]){
				//If the char is int the lowercase range; convert.
				if (buf[i] < 123 && buf[i] > 96){
					//putc(toupper(buf[i]));
					write(1, toupper(buf[i], 1);
				}
				//COME BACK! THIS MIGHT BE CAUSING ERRORS
				else if (buf[i] == "\r" || buf[i] == "\n"){

					//putc(buf[i]);
					//putc("\n");
				}
				else{	//Print he other ocharacters
					write(1, &buf[i], 1);
					//piutc(buf[i]);
				}
				i++;
			}
		}
	}
	//In the case you want to display the file to console in upper case
	if (argc == 2){
		infile = open(argv[1], O_RDONLY);

		//Read through the file
		while ((readIn = read(infile, buf, 128)) > 0){
			i = 0;
			//Read through the line
			while(buf[i]){
				//Convert
				if (buf[i] < 123 && buf[i] > 96){
					buf[i] = toupper(buf[i]);
					write(1, &buf[i], 1);
				}
				else if (buf[i] == '\n'){
					write(2, "\r", 1);
					write(1,"\n",1);
				}
				else
					write(1, &buf[i], 1);
				i++;
			}
		}

	}
	//In the case you want to convert the file to uppercase and store in a file
	if (argc >= 3){
		infile = open(argv[1], O_RDONLY);
		outfile = open(argv[2], O_WRONLY | O_CREAT);
		//Same as before
		while ((readIn = read(infile, buf, 128)) > 0){
			i = 0;
			while(buf[i]){
				if (buf[i] < 123 && buf[i] > 96){
					buf[i] = toupper(buf[i]);
					write(outfile, &buf[i], 1);
				}
				else if (buf[i] == "\r")
					write(outfile,"\n",1);
				else
					write(outfile, &buf[i], 1);
				i++;
			}
		}
		
		close(infile);
		close(outfile);
	}


	exit(0);
}
