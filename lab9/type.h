typedef unsigned char   u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#define NULL     0
#define NPROC    9
#define SSIZE 1024

// serial.c file for SERIAL LAB ASSIGNEMNT
/**************** CONSTANTS ***********************/
#define BUFLEN      64
#define NULLCHAR     0

#define NR_STTY      2    /* number of serial ports */

/* offset from serial ports base */
#define DATA         0   /* Data reg for Rx, Tx   */
#define DIVL         0   /* When used as divisor  */
#define DIVH         1   /* to generate baud rate */
#define IER          1   /* Interrupt Enable reg  */
#define IIR          2   /* Interrupt ID rer      */
#define LCR          3   /* Line Control reg      */
#define MCR          4   /* Modem Control reg     */
#define LSR          5   /* Line Status reg       */
#define MSR          6   /* Modem Status reg      */
#define BEEP 		 7

/******* PROC status ********/
#define FREE     0
#define READY    1
#define RUNNING  2
#define STOPPED  3
#define SLEEP    4
#define ZOMBIE   5

#define READ_PIPE  4
#define WRITE_PIPE 5

#define NOFT      20
#define NFD       10

#define PSIZE 10
#define NPIPE 10

#define BROKEN_PIPE -1

typedef struct pipe{
  char  buf[PSIZE];
  int   head, tail, data, room;
  int   nreader, nwriter;
  int   busy;
}PIPE;

typedef struct Oft{
  int   mode;
  int   refCount;
  struct pipe *pipe_ptr;
}OFT;

typedef struct proc{
    struct proc *next;
    int    *ksp;
    int    uss, usp;
    int    inkmode;
    int    pid;                // add pid for identify the proc
    int    status;             // status = FREE|READY|RUNNING|SLEEP|ZOMBIE    
    int    ppid;               // parent pid
    struct proc *parent;
    int    priority;
    int    event;
    int    exitCode;
    char   name[32];
	int time;

    OFT *fd[NFD];
    
    int    kstack[SSIZE];      // per proc stack area
}PROC;

struct semaphore{
  int value;
  PROC *queue;
};

struct stty {
   /* input buffer */
   char inbuf[BUFLEN];
   int inhead, intail;
   struct semaphore inchars;

   /* output buffer */
   char outbuf[BUFLEN];
   int outhead, outtail;
   struct semaphore outroom;
   int tx_on;

   /* Control section */
   char echo;   /* echo inputs */
   char ison;   /* on or off */
   char erase, kill, intr, quit, x_on, x_off, eof;
   
   /* I/O port base address */
   int port;
}stty[NR_STTY];
