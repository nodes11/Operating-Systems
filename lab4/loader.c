#include "ext2.h"


typedef struct header
{
     u32 ID_space;         // 0x04100301:combined I&D or 0x04200301:separate I&D
     u32 magic_number;     // 0x00000020
     u32 tsize;            // code section size in bytes
     u32 dsize;            // initialized data section size in bytes
     u32 bsize;            // bss section size in bytes
     u32 zero;             // 0
     u32 total_size;       // total memory size, including heap
     u32 symbolTable_size; // only if symbol table is present
} HEADER;

char buffer[1024], buffer2[1024];

int get_block(u16 blk, char *buf)
{ 
	diskr( blk/18, ((2*blk)%36)/18, (((2*blk)%36)%18), buf);
}

int search(INODE *ip, char *filename)
{ 
	int i = 0;
	char *cp;
	DIR *dp;
	for (i = 0; i < 12 && ip->i_block[i] != 0; i++)
	{	
		if (ip->i_block[i] == 0)  { return 0; }
		get_block((u16)ip->i_block[i], buffer2); 
		dp = (DIR *)buffer2;	
		cp = buffer2;
		while (cp < buffer2 + 1024)		
		{	
			if (strncmp(filename, dp->name, dp->name_len) == 0 && strlen(filename) == dp->name_len)
			{
		      		return dp->inode;
			}
		   	cp += dp->rec_len;         
		   	dp = (DIR *)cp;
		}
	}
	return 0;
}

int getino(char *filename)
{
	int ino = 2, inbb = 0; 
	char *token;
	INODE *ip; 
	GD *gp; 

	// get group descriptor block
	get_block(2, buffer); 
	gp = (GD *)buffer; 
	inbb = gp->bg_inode_table;				/* get the inode begin block		*/

	// get the root				
 	get_block(((ino - 1) / 8) + inbb, buffer); 		/* get the block with root inode	*/
	ip = (INODE *)buffer + (ino - 1) % 8;      		/* get the inode by offset		*/
	token = strtok(filename, "/"); 				/* now tokenize the string 		*/
	while (token  != 0)
	{
		ino = search(ip, token); 			/* search for each token in order /a - /b - /c etc 	*/
		if (!ino) { break; }				/* if you reached the end -> or dne 			*/
		get_block(((ino - 1) / 8) + inbb, buffer);
		ip = (INODE *)buffer + (ino - 1) % 8; 
		token = strtok(0, "/");
	}
	return ino;
}

load(char *filename, u16 segment)
{	
	INODE *ip; 
	GD *gp; 
	HEADER *hp; 
	u32 *intp;
	char name[32];
	int ino, inbb, idb;
	int i;						
	long tsize, dsize, bsize;
	
	strcpy(name, filename);

	get_block(2, buffer); 			
	gp = (GD *)buffer; 
	inbb = (u16)gp->bg_inode_table;	

	ino = getino(name); 	

	get_block(((ino - 1) / 8) + inbb, buffer);
	ip = (INODE *)buffer + (ino - 1) % 8;

	if ((u16)ip->i_block[12]) { get_block((u16)ip->i_block[12], buffer2); } 

	setes(segment);

	for (i = 0; i < 12; i++)
	{
		get_block((u16)ip->i_block[i],i*1024);
	}
	i = 12;
	if ((u16)ip->i_block[12])
	{
		intp = (u32 *)buffer2;
		while(*intp)
		{
			get_block((u16)*intp,i*1024);
			intp++;
			i++;
		}
	}

	setes(0x1000);
}
































































