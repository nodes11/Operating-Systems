//typedef unsigned int u32;

int BASE = 10;

char *table = "0123456789ABCDEF";

void gets(char str[]){
	while((*str = getc()) != '\r'){
		putc(*str++);
	}

	*str = 0;
}

int printf(char *fmt, ...){
	/*cp will point to fmt, ip will point to each parameter, and ebp is the stack pointer*/
	char *cp;
	int *ip;

	/*cp now points at fmt, and ip points at the first parameter*/
	cp = fmt;	//now points at fmt (8 bytes up)
	ip = (int *)(&fmt+1);	//according to the stack diagram the variables start 12 bytes up from the FP/ebp
		//^ebp returns as an int, but we need an int *

	while (*cp){
		if (*cp == '%'){
			cp++;	//Move to address of the next char
			if (*cp	== 'c')	//Char (don't need?)
				putc(*ip);	//pass the dereferenced value
			else if (*cp == 'x')	//Hex
				printx(*ip);
			else if (*cp == 'd')	//Decimal
				printd(*ip);
			else if (*cp == 'u')	//Unsigned
				printu(*ip);
			else if (*cp == 's')	//String
				prints(*ip);
			else if (*cp == 'o')	//Octal
				printo(*ip);

			ip++;	//move to next address
		}
		else{	//We will place the character otherwise
			if (*cp == '\n'){
				putc(*cp);
				putc('\r');
			}
			else{
				putc(*cp);
			}
		}

		cp++;	//Move to the next char type
	}

}

/*************Print string function****************/
int prints(char* str)
{
	while(*str){
		if (*str != '\n'){	//If the char does not contain a newline char, then print normally	
			putc(*str);
		}
		else{	//If we encounter a newline, the insert a '\r'
			putc(*str);
			putc('\r');
		}

		str++;
	}
}

/*************From K.C.**************************/
int rpu(u32 x)
{
	char c;
	x+48;
	if (x){
		c = table[x % BASE];
		rpu(x / BASE);
		putc(c);
	}
} 

/***********From K.C.**************************/
int printu(u32 x)
{
	BASE = 10;
	if (x==0)
		putc('0');
	else
		rpu(x);
		putc(' ');
		
	return 0;
}

/**********printd function********************/
int printd(int x)
{
	BASE = 10;
	if (x == 0){
		putc('0');
	}else if(x < 0){
		putc('-');
		x = -x;
	}
	rpu(x);

	return 0;
}

/**********printo function******************/
int printo(u32 x){
	BASE = 8; //Octal uses base 8 (0-7)
	putc('0');

	rpu(x);
	
	return 0;
}

/*********printx function*****************/
int printx(u32 x){
	BASE = 16;

	putc('0');	//start with 0x
	putc('x');
	
	rpu(x);	//Call the recursive print
	
	return 0;
}
