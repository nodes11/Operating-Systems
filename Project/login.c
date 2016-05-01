#include "ucode.c"

char *tty;
char *users[128];
int in, out, err;

/*split the env path into tokens*/
void parseUser(char *line, char *users[]){
	char *s;
	int i = 0;

	s = strtok(line, ":");
	users[i] = s;

	//Split up the rest of the items in the path. Add them to the array of parts
	while ((s = strtok(0, ":")))
	{
		users[++i] = s;	
	}
}

int main(int argc, char *argv[]){
	char username[64],
		 password[64],
		 buf[1024],
		 tbuf[1024];
	int pwdFile,
		readIn,
		i, j,
		userSet = 0;
	STAT *pwdFileInfo;
	
	tty = argv[1];

	close(0);
	close(1);
	close(2);

	//Open up 0,1,2 on the console
	in = open(tty, 0);
	out = open(tty, 1);
	err = open(tty, 2);

	settty(tty);

	//Open the password file and stat the file to get the size
	//pwdFile = open("/etc/passwd", 0);
	//pwdFileInfo = stat("/etc/passwd");

	//Print our our welcome message
	printf("460 SHELL LOGIN: open %s as stdin, stdout, stderr\n", tty);
	printf("\n-----------------------------------------\n");

	//Now we want to get the user login info and check it against the file
	while(1){
		//Clear input buffers
		bzero(username, sizeof(username));
		bzero(password, sizeof(password));
		bzero(buf, sizeof(buf));
		bzero(tbuf, sizeof(tbuf));

		//Get the login and the password
		printf("login: ");
		gets(username);
		printf("password: ");
		gets(password);
		printf("\n-----------------------------------------\n");

		//Null termiante the strings
		username[strlen(username)] = 0;
		password[strlen(password)] = 0;

		//Open the password file and stat the file to get the size
		pwdFile = open("/etc/passwd", 0);
		pwdFileInfo = stat("/etc/passwd");

		//Check how big the file is
		readIn = pwdFileInfo->st_size;

		//Read until we have read
		while(readIn > 0){
			/******************
			 * How much do we want to read?
			 * 1. Is the file bigger than one block? Read 1k bytes
			 * 2. Or is the file smaller than 1K bytes? Read the size of the file
			 * ***************/
			if (pwdFileInfo->st_size < 1024){
				read(pwdFile, buf, pwdFileInfo->st_size);
				readIn -= pwdFileInfo->st_size;
			}
			else{
				read(pwdFile, buf, 1024);
				readIn -= 1024;
			}

			i = 0;
			j = 0;
			while(buf[i]){
				//Have we reached the end of a line?
				if (buf[i] == 10){
					j = 0;
					
					//Parse the user
					parseUser(&tbuf, users);

					//Check for a valid user and login if valid
					if ((strcmp(users[0], username) == 0) && (strcmp(users[1], password) == 0)){
						printf("Logging in as %s\n", users[0]);
						
						chuid(atoi(users[2]), atoi(users[3]));
						chdir(users[5]);

						printf("%s %d\n", users[6], strlen(users[6]));

						exec(users[6]);
					}

					//Clear the temporary buffer (stores a single user at a time)
					while(tbuf[i] != 0){
						tbuf[i++] = 0;
					}
				}
				else{
					tbuf[j] = buf[i];
					j++;
				}
				i++;
			}

			close(pwdFile);
		}
	}
}
