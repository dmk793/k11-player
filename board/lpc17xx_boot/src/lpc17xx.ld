/*
 *
 */
MEMORY
{
    /* limit 64k sram memory block of device to memory only used by DPort */
    RAM      (rwx)    : ORIGIN = 0x10000000, LENGTH = 8K
    /*
     * bootloader copy vector table to SRAM - reserve some space.
     * 256 bytes should be enough (168 bytes actually, 40 vectors,
     * 4 bytes for stack pointer, 4 bytes for reset vector).
     */
    VECTRAM  (rwx)    : ORIGIN = 0x20000000, LENGTH = 256
    /* four user-defined vectors that can be called from DPort interface */
    UVECTRAM (rwx)    : ORIGIN = 0x20000100, LENGTH = 16
    RAM0_0   (rwx)    : ORIGIN = 0x20000104, LENGTH = 7936
    RAM0_1   (rwx)    : ORIGIN = 0x20002000, LENGTH = 8K
    RAM1     (rwx)    : ORIGIN = 0x20004000, LENGTH = 16K
    /* NOTE 512 Kb available, but limit to 32k */
    FLASH (rx)        : ORIGIN = 0x00000000, LENGTH = 32K
    FLASH_TRUE (rx)   : ORIGIN = 0x00000000, LENGTH = 512K
}

_estack = ORIGIN(RAM)+LENGTH(RAM);

_flash_start = ORIGIN(FLASH_TRUE);
_flash_size  = LENGTH(FLASH_TRUE);

_vect_start  = ORIGIN(VECTRAM);
_vect_size   = LENGTH(VECTRAM);

_uvect_start = ORIGIN(UVECTRAM);
_uvect_size  = LENGTH(UVECTRAM);

SECTIONS
{
    .text : {
	. = ALIGN(4);
	*(vectors)      /* Vector table */
	*(.text)        /* Program code */
        *(.text.*)      /* remaining code */
        *(.rodata)      /* read-only data (constants) */
        *(.rodata*)
 
	. = ALIGN(4);
   	 _etext = .;
	    /* This is used by the startup in order to initialize the .data secion */
   	 _sidata = _etext;
    } >FLASH

    /* .ARM.exidx is sorted, so has to go in its own output section. 
    .ARM.exidx :
    {
      *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } >RAM
    */

    /DISCARD/ :
    {
      *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    }
   
    /* This is the initialized data section
    The program executes knowing that the data is in the RAM
    but the loader puts the initial values in the FLASH (inidata).
    It is one task of the startup to copy the initial values from FLASH to RAM. */
    .data  : AT ( _sidata )
    {
	. = ALIGN(4);
        /* This is used by the startup in order to initialize the .data secion */
        _sdata = . ;
        
        *(.data)
        *(.data.*)

	. = ALIGN(4);
	/* This is used by the startup in order to initialize the .data secion */
   	 _edata = . ;
    } >RAM
    
    /* This is the uninitialized data section */
    .bss (NOLOAD):
    {
	. = ALIGN(4);
        /* This is used by the startup in order to initialize the .bss secion */
        _sbss = .;
        
        *(.bss)
        *(COMMON)
        
	. = ALIGN(4);
	/* This is used by the startup in order to initialize the .bss secion */
   	 _ebss = . ;
    } >RAM

    PROVIDE ( end = _ebss );
    PROVIDE ( _end = _ebss );
}  

PROVIDE( _heap = _ebss );
PROVIDE ( _eheap = ALIGN(ORIGIN(RAM) + LENGTH(RAM) - 8 ,8) );

