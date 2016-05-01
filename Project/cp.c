#include "ucode.c"

int main(int argc, int *argv[]){
	int infile, outfile,
		readIn, buf, dbuf[2048];
	DIR *currD;
	STAT fileType;

	if (argc < 3){
		printf("Error: cp needs 3 arguments.\n");
	}
	else{
		infile = open(argv[1], O_RDONLY);
		if (infile < 0){
			printf("Invalid input file\n");
			return;
		}
		outfile = open(argv[2], O_WRONLY|O_CREAT);


		stat(infile, &fileType);

		//printf("Stat mode: %d\n", (fileType.st_mode && 40000));

		//if ((fileType.st_mode & 0100000) == 1){	//We have file
		//readIn = read(infile, &buf, 1);	
		while((readIn = read(infile, &buf, 1)) > 0){
			write(outfile, &buf, 1);
		}
		//}
		//eelse{	//We have a directory
			//Implement dir to dir
		//	printf("DIR TO DIR in progress\n"); 
		//}
		close(infile);
		close(outfile);
	}
}
