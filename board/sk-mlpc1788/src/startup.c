/* 
 *     This file is part of K11, hardware multimedia player.
 * 
 * Copyright (C) 2014 Dmitry Kobylin
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "startup.h"

extern unsigned long _sidata; /* start address for the initialization values of the .data section. defined in linker script */
extern unsigned long _sdata;  /* start address for the .data section. defined in linker script */
extern unsigned long _edata;  /* end address for the .data section. defined in linker script */

extern unsigned long _sbss; /* start address for the .bss section. defined in linker script */
extern unsigned long _ebss; /* end address for the .bss section. defined in linker script */

extern unsigned int *myvectors[];

int main(void);

/*
 *
 */
void __attribute__((naked)) Reset_Handler()
{
    unsigned long *pulSrc, *pulDest;

//    /*
//     * NOTE
//     * Initialization below no necessary if this
//     * handler is called by bootloader or processor itself.
//     *
//     * But if handler is called from debugger we should:
//     *    * set MSP as thread stack pointer
//     *    * initialize SP from vector table
//     */
//    asm volatile (
//        /* set MSP as thread stack pointer, unprivileged thread mode */
//        "mov r0,#0        \n"
//        "msr control, r0  \n"
//        "mov r0, %0       \n"
//        "msr msp, r0      \n"
//        : : "r"(myvectors[0]) : "r0"
//    );

    /* copy the data segment initializers from flash to SRAM */
    pulSrc = &_sidata;

    /* NOTE no need to initialize if code performed from RAM (already copied by bootloader) */
    if (_sidata != _sdata)
    {
        for(pulDest = &_sdata; pulDest < &_edata; )
        {
            *(pulDest++) = *(pulSrc++);
        }
    }

    /* zero fill the bss segment */
    for(pulDest = &_sbss; pulDest < &_ebss; )
    {
        *(pulDest++) = 0;
    }

    /* call the application's entry point */
    main();

    /* NOTREACHED */
    for(;;);
}

