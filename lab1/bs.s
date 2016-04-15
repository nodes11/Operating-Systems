!---------------------- bs.s file -------------------------------------------
!-     The CPU's internal registers are
!-              segment registers: CS, DS, SS, ES 
!-              general registers: AX, BX, CX, DX, BP, SI, DI
!-              status  register : FLAG
!-              stack pointer    : SP
!-              instruction point or program counter: IP
!-     All registers are 16-bit wide. 
!----------------------------------------------------------------------------
		BOOTSEG  = 0x9000
		SSP      = 32*1024
	
        .globl _main, _prints,_color,_dap        ! IMPORT 
        .globl _diskr,_getc,_putc,_error         ! EXPORT
	
        !-------------------------------------------------------------------
        ! MBR loaded at segment 0x07C0. Load entire booter to segment 0x9000 
        !-------------------------------------------------------------------
        mov  ax,#BOOTSEG
        mov  es,ax			! load temp area into ax
        xor  bx,bx          ! clear BX = 0	|	MBR is now stored in ES
        !---------------------------------------------------
        !  read entire booter in sector#1-8KB to 0x9000
        !---------------------------------------------------
        mov  dx,#0x0080     ! dh=head=0, dL=0x80=HD or USB's MBR
        xor  cx,cx			! clear cx
        incb cl             ! cyl 0, sector 1 (increment)
		incb cl             ! cyl 0, sector 2 (increment)
        mov  ax, #0x0220    ! READ 16 sectors (booter<8KB)
        int  0x13

        jmpi next,BOOTSEG   ! CS=BOOTSEG, IP=next	(call next function)

next:
        mov  ax,cs          ! establish segments again (move ax to code segment)
        mov  ds,ax          ! we know ES,CS=0x9000. Let DS=CS  (move data segment to ax register)
        mov  ss,ax			! move stack segment to ax register
        mov  es,ax			! move extra segement (temp space) to ax reister
        mov  sp,#SSP        ! 32 KB stack (move stack pointer up)


!
!                
!   0x1000                           0x9000                        0xA000  
!   ---------------------------------|---------------|-------------|----------
!   |   |//|                         |Code|Data|stack|             |  ROM
!   ----|----------------------------|---------------|-------------|----------
!       0x7C00                       CS              sp=32KB
!                                    DS              from SS
!                                    SS 
!

!-------------------------------------------------------------	
        mov  ax,#0x0012     ! Call BIOS for 640x480 color mode     
		int  0x10
!------------------------------------------------------------
        call _main          ! call main() in C
     
        test ax, ax         ! check return value from main()
        je   _error         ! main() return 0 if error

        jmpi 0, 0x1000

        !------------------------------------------------------
        !  char getc( )   function: return a char from keyboard
        !------------------------------------------------------
_getc:
        xorb   ah,ah           ! clear ah
        int    0x16            ! call BIOS to get a char in AX
        andb   al,#0x7F        ! 7-bit ascii  
        ret 
	
        !---------------------------------------------------
        ! int putc(char c)  function: print a char to screen
        !---------------------------------------------------
_putc:           
        push   bp
		mov    bp,sp

        movb   al,4[bp]        ! get the char into aL
        movb   ah,#14          ! aH = 14
        mov    bx,_color       ! cyan
        int    0x10            ! call BIOS to display the char

        pop    bp
	ret
        
        !------------------------------
        !       error() & reboot
        !------------------------------
_error:
        push #msg
        call _prints
        int  0x19               ! reboot
	
msg:   .asciz  "\n\rError!\n\r"

!setds(segment) : set DS to segment to R/W memory outside BOOTSEG 
!_setds:
	push  bp
	mov   bp,sp
	mov   ax,4[bp]
	mov   ds,ax
	pop   bp
	ret
	
! diskr(): read disk sectors specified by _dap in C code		
_diskr: 
        mov dx, #0x0080
	 	mov ax, #0x4200
	 	mov si, #_dap

        int 0x13               ! call BIOS to read the block 
        jb  _error             ! to error() if CarryBit is on [read failed]
	ret
