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

/*
 *
 */
#include <LPC177x_8x.h>
#include <uart.h>
#include <crc.h>
#include <debug.h>
#include <stimer.h>
#include <gpio.h>
#include "sdram.h"
#include "version.h"
#include "mpu.h"
#include "dport.h"

static void bootjump(uint32 address);
static void start_main();

extern uint32 * _vect_start;
extern uint32 * _vect_size;

const struct gpio_t testpin = {
    LPC_GPIO0, (1 << 21) /* MP118    P0.21 */
};

/*
 *
 */
int main(void)
{
    gpio_init();
    stimer_init(); /* NOTE used by dport, move code to dport itself */
    dport_init();
    uart_setup(DEBUG_UART, UART_BAUD_115200, NULL);

    dprint("nsn", "*********************************");
    dprint("sn",  "* "MODULE_NAME);
    dprint("sn",  "* v"VERSION_STRING", "__DATE__" "__TIME__);
    dprint("sn",  "*********************************");

    dprint("<CLK_CCLK = >g4d< MHz>n", 3, CLK_CCLK / 1000 / 1000);
    dprint("<CLK_PCLK = >g4d< MHz>n", 3, CLK_PCLK / 1000 / 1000);
    dprint("<CLK_EMC  = >g4d< MHz>n", 3, CLK_EMC  / 1000 / 1000);

    mpu_init();
    sdram_init();

    if (!sdram_test(gpio_read(&testpin) ? SDRAM_TEST_SHORT : SDRAM_TEST_FULL))
        while (1);

    /* check whether we should copy main program to SDRAM */
    if (LOAD_OFFSET >= SDRAM_BASE_ADDR && LOAD_OFFSET < (SDRAM_BASE_ADDR + SDRAM_SIZE))
    {
        /* copy main program to SDRAM */
        {
#if 1
            /*
             * Following code is used for loading main program by dport directly to SDRAM.
             * If RTC general purpose register contains magic number, skip load of main program from flash.
             * "bkpt" instruction is used for entry to dport handler.
             */
            dprint("s4xn", "MAGIC   ", LPC_RTC->GPREG0);
            if (LPC_RTC->GPREG0 == 0x4444BEEF)
            {
                dprint("<wait debugger>n", 0);
                asm volatile (
                    "bkpt #0xAB \n"
                    "nop        \n"
                    "cpsid i    \n"
                    "dsb        \n"
                    "isb        \n"
                );
                /* NOTE spoil value to prevent entry to handler each time. */
                LPC_RTC->GPREG0 = 0x00000000;
            } else
#endif
            {
                uint8 *rd;
                uint8 *wr;
                uint32 len;
                uint16 crc, fcrc;

                len = *((uint32*)SIZE_OFFSET);
                crc = *((uint16*)CRC_OFFSET);

                dprint("<LENGTH  >4xn", len);

                /* sanity check */
                if (len > SDRAM_SIZE)
                {
                    dprint("sn", ERR_PREFIX "size too long");
                    for (;;); /* NOTE loop */
                }

                rd = (uint8*)IMAGE_OFFSET;
                wr = (uint8*)LOAD_OFFSET;

                /* check CRC */
                crc16ccitt_init(&fcrc);
                crc16ccitt_update(&fcrc, rd, len);
                dprint("<CRC     >2xn", crc);
                dprint("<FCRC    >2xn", fcrc);
                if (fcrc != crc)
                {
                    dprint("sn", ERR_PREFIX "checksum mismatch");
                    for (;;); /* NOTE loop */
                }

                while (len--)
                    *wr++ = *rd++;
                asm volatile (
                    "dsb \n"
                    "isb \n"
                );
            }
        }

        /* copy vector table of main program from SDRAM to SRAM */
        {
            uint32 len;
            uint8* rd;
            uint8* wr;

            len = (uint32)&_vect_size;
            rd  = (uint8*)LOAD_OFFSET;
            wr  = (uint8*)&_vect_start;

            while (len--)
                *wr++ = *rd++;
            asm volatile (
                "dsb \n"
                "isb \n"
            );
        }
    }

    start_main();

    /* UNREACHED */
    for (;;);

    return 0;
}

/*
 *
 */
static void start_main()
{
    dprint("<LOAD    >4xn", LOAD_OFFSET);

    __disable_irq();
    /*
     * NOTE
     * SDRAM can not be used as vector table (?).
     *
     * In order of loading of main program to SDRAM by dport
     * we should use vector table from SRAM.
     */
    SCB->VTOR = ((uint32)&_vect_start);
    /* NOTE irq stay disabled */

    asm volatile ("dsb");
    asm volatile ("isb");
    bootjump(LOAD_OFFSET);
}

/*
 *
 */
static void __attribute__((naked)) bootjump(uint32 address)
{
    asm volatile (
        "ldr r1, [r0,#0]      \n\t"
        "msr msp, r1          \n\t"
        "ldr PC,[r0,#4]       \n\t"
    );
}

///*
// *
// * NOTE look for debug port table offsets in "dport/src/it.c" file
// */
//static void dport_startup()
//{
//    int i;
//    unsigned int *src, *dst;
//    void *init();
//
//    /* copy debug port image to it's location in SRAM */
//    src = (unsigned int*)DPORT_IMAGE_ADDR;
//    dst = (unsigned int*)DPORT_LOAD_ADDR;
//
//    /* check magic number */
//    if (*src != DPORT_VECTOR_MAGIC)
//        return;
//
//#ifdef CHECK_MAGIC
//    #define IMAGE_SIZE ((DPORT_IMAGE_SIZE / 4) - 4)
//#else
//    #define IMAGE_SIZE (DPORT_IMAGE_SIZE / 4)
//#endif
//
//    /* XXX image size should be multiple of sizeof(unsigned int) */
//    for (i = 0; i < IMAGE_SIZE; i++)
//        *dst++ = *src++;
//
//    /* call initialization function */
//    asm volatile(
//        "mov r1, %0     \n"
//        "ldr r0, [r1,#0]\n"
//        "blx r0         \n"
//        : : "r" (DPORT_IMAGE_ADDR + 4) : "r0", "r1"
//    );
//}
//
