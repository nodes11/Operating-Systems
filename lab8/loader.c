#include "ext2.h"
#define BLOCK_SIZE 1024

typedef struct header{
     u32 ID_space;         // 0x04100301:combined I&D or 0x04200301:separate I&D
     u32 magic_number;     // 0x00000020
     u32 tsize;            // code section size in bytes
     u32 dsize;            // initialized data section size in bytes
     u32 bsize;            // bss section size in bytes
     u32 zero;             // 0
     u32 total_size;       // total memory size, including heap
     u32 symbolTable_size; // only if symbol table is present
} HEADER;

char buf[1024];
char ibuf[1024];
int tsize, dsize, bsize;

//Load the block into the buf
int get_block(u16 blk, char *buf)
{
	diskr(blk/18, ((2*blk)%36)/18, (((2*blk)%36)%18), buf);
}

int getino(char *findpath)
{
  //Used for tokenizing
  char *token;
  char *names[32];
  
  //Indexes and values
  int name_count = 0;
  int inn = 2, beginBlock, i;

  //INODES and Blocks
  INODE *tInode;
  GD *gp;

  //Get the group descriptor block
  get_block(2, buf); 
  gp = (GD *)buf;

  //Get the block where the inodes start
  beginBlock = gp->bg_inode_table;

  //Check for an empty path
  if((findpath[0] == '/' && strlen(findpath) == 1) || findpath[0] == '\0')
  {
    names[name_count] = ".";
  }
  else{
    //Get the first token from the path and store it
    token = strtok(findpath, "/");
    names[name_count] = token;

    //Tokenize the path until we get every piece in the array
    while(0 != (token = strtok(0, "/")))
    {
      name_count++;
      names[name_count] = token;
    }
  }

  //Get the root inode
  get_block(((inn-1)/8)+beginBlock, buf);
  tInode = (INODE *)buf + (inn-1)%8;

  // Loops through the pathname array, breaks out if name cannot be found
  for(i = 0; i <= name_count; i++)
  {
    int offset, block;

    inn = search(names[i], tInode);

    if(0 == inn)  // This will exit if a pathname was incorrectly given.
    {
      return 0;
    }

    //mailman's algorithm on inn
    block = (inn - 1) / 8 + beginBlock;
    offset = (inn - 1) % 8;

    //reassign INODE *ip to new inode
    get_block(block, buf);
    tInode = (INODE *)buf + offset;
  }

  return inn;
}


// This looks through data blocks of inode structure
// This function is for getino to locate the correct inode number.
int search(char *name, INODE *tInode)
{
  DIR *dp = (DIR *) buf;
  char *cp = buf;
  int i;

  for(i = 0; i < 12; i++)
  {
    if(tInode->i_block[i] == 0) //This means no more data blocks
    {
      return 0;
    }
    else{
      get_block((u16)tInode->i_block[i], buf);

      while(cp < buf + BLOCK_SIZE)
      {
        dp->name[dp->name_len] = '\0'; //null terminates name of dir entry

        if(0 == strcmp(name, dp->name))
        {
          return dp->inode;
        }

        cp += dp->rec_len;
        dp = (DIR *) cp;
      }
    }
  }
  return 0;
}

load(char *filename, u16 segment)
{	
	INODE *ip; 
	GD *gp; 
	HEADER *hp; 
	
	char name[32];

	int ino, beginBlock, idb;
	int i,j;						
	long tsize, dsize, bsize;

	//Points to the proper block for the indirect blocks
	u32 *idptr;

	//Copy the filename into a char array
	strcpy(name, filename);

	//Get the group descriptor block
	get_block(2, buf); 			
	gp = (GD *)buf;
	
	//Set the beginBlock so we know where the inodes start
	beginBlock = gp->bg_inode_table;	

	//Get the inode of the filepath/file
	ino = getino(name);	
	//If there is a problem finding the file	
	if (ino == 0){
		printf("ERROR: File Not Found!\n");
		return 0;
	}

	//There are 8 inodes per block... Use mailmans
	get_block(((ino - 1) / 8) + beginBlock, buf);
	//Get the root inode
	ip = (INODE *)buf + (ino - 1) % 8;

	//Load the header information
	get_block((u16)ip->i_block[0], ibuf);
	hp = (HEADER *)ibuf;

	//Print out the header information and store it into the global values
	printf("Header information:\n");
	printf("Code Size:\t%d\n", (u32)hp->tsize);
	tsize = (u32)hp->tsize;
	printf("Data Size:\t%d\n", (u32)hp->dsize);
	dsize = (u32)hp->dsize;
	printf("BSS Size:\t%d\n\n", (u32)hp->bsize);
	bsize = (u32)hp->bsize;

	//Check for indirect blocks... Load to second buffer if existant
	if ((u16)ip->i_block[12]){
		get_block((u16)ip->i_block[12], ibuf); 
	} 

	//0x1000*{2-8} -> the start of umode segment
	//es points to the segment we're reading from
	//bx is offset from that segment
	setes(segment);

	//Load the direct blocks (loading code and data)
	for (i = 0; i < 12; i++)
	{
		if ((u16)ip->i_block[i]){
			get_block((u16)ip->i_block[i], 0);	//Read in the direct block
			
			inces();	//Move the es 1024 bytes to the next segment
			
			putc('*');	//Print a "*" for every indirect block read in
		}
	}

	//Load the indirect blocks if existant
	if ((u16)ip->i_block[12])
	{
		idptr = (u32 *)ibuf;
		while(*idptr)
		{
			get_block((u16)*idptr,0);	//Read in the block
			
			idptr++;			//0-256 indirect blocks. Increment for every get_block

			inces();			//Move the es 1024 bytes to the next segment

			putc("+");			//Print a "+" for every indirect block read in
		}

		putc('\n');
	}

	//Set es back to kmode segment
	setes(0x1000);

	//Shifting left to overwrite the header
	//the header is 32 bytes
	for (j = 0; j < tsize + dsize ; j++){
		put_word((u16)get_word(segment,j+32), segment, j);
	} 
	
	return 1;
}
