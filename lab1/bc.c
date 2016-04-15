typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#include "bio.c"

#define  GREEN  10         // color byte for putc()
#define  CYAN   11
#define  RED    12
#define  MAG    13
#define  YELLOW 14

struct partition {         // Partition table entry in MBR
       u8  drive;          // 0x80 - active 
       u8  head;	   // starting head 
       u8  sector;	   // starting sector 
       u8  cylinder;       // starting cylinder 
       u8  type;	   // partition type 
       u8  end_head;       // end head 
       u8  end_sector;	   // end sector 
       u8  end_cylinder;   // end cylinder 
       u32 start_sector;   // starting sector counting from 0 
       u32 nr_sectors;     // nr of sectors in partition 
};

struct dap{                // DAP for extended INT 13-42  
       u8   len;           // dap length=0x10 (16 bytes)
       u8   zero;          // must be 0  
       u16  nsector;       // number of sectors to read: 1 to 127
       u16  addr;          // memory address = (segment:addr)
       u16  segment;    
       u32  s1;            // low  4 bytes of sector#
       u32  s2;            // high 4 bytes of sector#
};

struct dap dap, *dp;       // global dap struct
u16 color = RED;           // initial color for putc()

#define  BOOTSEG 0x9000

//#include "bio.c" <========= WRITE YOUR OWN io.c file !!!!

char mbr[512];	//All partitions will be loaded into MBR, each sector is 512 bytes
char ans[64];	//Used for textual input (i.e. gets();)

// load a disk sector to (DS, addr), where addr is an offset in segment
//
// Loading the first sector of the disk into the device into Master Boot Record (mbr)
int getSector(u32 sector, char *addr)
{
  dp->addr    = addr;
  dp->s1      = (u32)sector;
  diskr();    // call int13-43 in assembly
}

// Give the MBR, we know that the partition table starts at the offset
// Works very similar to iterating through inode blocks
int showPartitions(){
	char *p = (mbr + 0x1BE);	//Set our starting pointer (use the offset)
	int pNum = 0;					//

	struct partition *cp = (struct partition *)p;

	
	/*A hard disk can be divided into 4 Primary Partitions. The partitions
	are recorded in a Partition Table in the MBR at 0x1BE. Each Partition
	Table entry is a structure as shown below.
	
	Each entry is 16 bytes (for a total of 64 bytes). The last 2 bytes of the
    boot sector contain the boot signature 0x55AA.*/


	while(pNum < 4){
		//Iterate through mbr using the nr_sectors as our iterator
		printf("---------------------------------------------\n");
		printf("\t\tPartition %d\n", pNum+1);
		printf("---------------------------------------------\n");
		
		//Print out partition elements
		printf("cp->drive:\t\t%d\n", cp->drive); 
       	printf("cp->head:\t\t%d\n", cp->head);	   
      	printf("cp->sector:\t\t%d\n", cp->sector);	   
       	printf("cp->cylinder:\t\t%d\n", cp->cylinder);       
       	printf("cp->type:\t\t%d\n", cp->type);	   
       	printf("cp->end_head:\t\t%d\n", cp->end_head);       
       	printf("cp->end_sector:\t\t%d\n", cp->end_sector);	   
       	printf("cp->end_cylinder:\t%d\n", cp->end_cylinder);   
      	printf("cp->start_sector:\t%d\n", cp->start_sector);  
       	printf("cp->nr_sectors:\t\t%d\n", cp->nr_sectors);  
		
		//Move to next partition segment
		cp++;

		//We know we have 4 partitions... So we'll count for them
		pNum++;

		//Pause before printing next one
		getc();
	}
		printf("---------------------------------------------\n");
		printf("\t\tEnd Partitions\n");
		printf("---------------------------------------------\n");
	
}

int main()
{
  int i;
  struct partition *p;
  printf("booter start in main()\n");

  // initialize the dap struct
  dp = &dap;
  dp->len  = 0x10;        // must be 16
  dp->zero = 0;           // must be 0
  dp->nsector = 1;        // load one sector
  dp->addr = 0;           // will set to addr              
  dp->segment = BOOTSEG;  // load to which segment
  dp->s1 = 0;             // will set to LBA sector#
  dp->s2 = 0;             // high 4-byte address s2=0

  getSector((u32)0, (u16)mbr); // get MBR
  /*==========================================================
  In the MBR, partition table begins at byte offset 0x1BE
  // DO #1:  WRITE C CODE TO PRINT THE PARTITIONS
  ==========================================================*/
  color = CYAN;

	showPartitions();

	getc();

  while(1){
    printf("what's your name? ");
    gets(ans);
    if (strcmp(ans, "quit")==0){
      printf("\nexit main()\n");
      break;
    }
    printf("\nWelcome %s!\n", ans);
  }
}
/*====================================================================
DO #2: The bs.s file contains
          char getc()         : return a char from keyboard
          int  putc(char c)   : display a char to screen  
   which are supported by BIOS. There are NO OTHER terminal I/O functions!!!

   Write your OWN gets(char s[ ]) fucntion to get a string.
   Write your OWN printf(char *fmt, ...) as in CS360

Then, include YOUR io.c file in bc.c. 
      Do NOT use KCW's io.o in linking step.
===================================================================*/
