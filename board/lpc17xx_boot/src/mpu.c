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

#include <LPC177x_8x.h>
#include "mpu.h"
#include "sdram.h"

/*
 *
 */
void mpu_init()
{
    /* setup access for SDRAM */

    __disable_irq();

    asm volatile("dmb\n");

    /* disable MPU */
    MPU->CTRL = 0;

    /* set first region parameters */
    {
        MPU->RNR = 0;

        /* normal, shareable, outer and inner write-back, write and read allocate */
#define RASR_TEX    1
#define RASR_C      1
#define RASR_B      1
#define RASR_S      1
//#define RASR_TEX    0
//#define RASR_C      0
//#define RASR_B      0
//#define RASR_S      0
        MPU->RBAR = SDRAM_BASE_ADDR & 0xffffffe0;
        MPU->RASR =
            (0        << 28)  |  /* instruction fetches enabled */
            (3        << 24)  |  /* full access by priveleged and unpriveleged software */
            (RASR_TEX << 19)  |
            (RASR_S   << 18)  |
            (RASR_C   << 17)  |
            (RASR_B   << 16)  |
            ((25 - 1)  << 1)  | /* XXX 32 Mb */
            (1 << 0);           /* enable region */  
    }

    /*
     * enable MPU,
     * enable use of default memory map,
     * disable MPU during hard fault
     */
    MPU->CTRL = (1 << 0) | (0 << 1) | (1 << 2);

    asm volatile(
        "dsb\n"
        "isb\n"
    );

    __enable_irq();
}

