/*
 *
 */
MEMORY
{
    /* first 8K of SRAM is used by DPort */
    DPORTRAM (rwx)    : ORIGIN = 0x10000000, LENGTH = 8K
    RAM      (rwx)    : ORIGIN = 0x10002000, LENGTH = 56K
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
    SDRAM    (rwx)    : ORIGIN = 0xA0000000, LENGTH = 32M
    FLASH    (rx)     : ORIGIN = 0x00000000, LENGTH = 512K
}

_estack = ORIGIN(SDRAM)+LENGTH(SDRAM);

_ethram      = ORIGIN(RAM1);
_ethram_size = LENGTH(RAM1);

_used_ram = ORIGIN(SDRAM);

_uvect_start = ORIGIN(UVECTRAM);
_uvect_size  = LENGTH(UVECTRAM);

SECTIONS
{
    /*
     *
     */
    .text LOAD_OFFSET : {
	. = ALIGN(4);
	*(vectors)      /* vector table, XXX used from flash anyway */
	*(.text)        /* program code */
        *(.text.*)      /* remaining code */
        *(.rodata)      /* read-only data (constants) */
        *(.rodata*)
 
	. = ALIGN(4);
   	_sidata   = .; /* start of data section */
        _data_vma = (LOAD_OFFSET >= _used_ram) ? . : (_used_ram - LOAD_OFFSET) /* XXX */;
    }

    /* This is the initialized data section. */
    .data _data_vma : AT ( _sidata ) {
	. = ALIGN(4);
        _sdata = . ; /* start of data section */
        *(.data)
        *(.data.*)

	. = ALIGN(4);
   	_edata  = . ; /* end of data section */
        _bss_vma = .;
    }

    /* This is the uninitialized data section. */
    .bss _bss_vma (NOLOAD) :
    {
	. = ALIGN(4);
        _sbss = .; /* start of bss section */
        
        *(.bss)
        *(COMMON)
        
	. = ALIGN(4);
   	_ebss = . ; /* end of bss section */
        _bss2_vma = .;
    }

    /* This is used for memory used by OS (stacks, memory spool). */
    .bss2 _bss2_vma (NOLOAD): {
	. = ALIGN(4);
        *(tstack);
        *(heap);
	. = ALIGN(8);
        *(gsmem);
    }

    .bss3 (NOLOAD) : {
        . = ALIGN(4);
        *(sram);
    } >RAM

    /* This is used by DPort for user-defined functions */
    .userf (NOLOAD): {
        . = ALIGN(4);
        *(userf);
    } >UVECTRAM

    /* .ARM.exidx is sorted, so has to go in its own output section. 
    .ARM.exidx :
    {
      *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } >SDRAM
    */

    /DISCARD/ :
    {
      *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    }

    /*
    PROVIDE ( end = _ebss );
    PROVIDE ( _end = _ebss );
    */
}  

/*
PROVIDE( _heap = _ebss );
PROVIDE ( _eheap = ALIGN(ORIGIN(SDRAM) + LENGTH(SDRAM) - 8 ,8) );
*/

