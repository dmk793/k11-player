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
#include <debug.h>
#include <stimer.h>
#include "sdram.h"

//#define ROW_BANK_COLUMN_MAP
#ifndef ROW_BANK_COLUMN_MAP
    #define BANK_ROW_COLUMN_MAP
#endif

/*
 * setup EMC to drive SDRAM
 */
void  __attribute__((optimize("O0"))) sdram_init()
{
    /*
     * SDRAM pins:
     *
     * CAS             P2[16]
     * RAS             P2[17]        
     * CLK             P2[18]
     * CS              P2[20]
     * CKE             P2[24]
     * DML             P2[28]
     * DMH             P2[29]
     * D0 .. D15       P3[0] .. P3[15]
     * WE              P4[25]
     * A0 .. A14       P4[0] .. P4[14]
     *
     */
//    volatile uint32_t i;
    volatile unsigned long Dummy;
    LPC_IOCON->P2_16 = 0x21;
    LPC_IOCON->P2_17 = 0x21;
    LPC_IOCON->P2_18 = 0x21;
    LPC_IOCON->P2_20 = 0x21;
    LPC_IOCON->P2_24 = 0x21;
    LPC_IOCON->P2_28 = 0x21;
    LPC_IOCON->P2_29 = 0x21; //
//    LPC_IOCON->P2_30 = 0x21; //
//    LPC_IOCON->P2_31 = 0x21;
    LPC_IOCON->P3_0 = 0x21;
    LPC_IOCON->P3_1 = 0x21;
    LPC_IOCON->P3_2 = 0x21;
    LPC_IOCON->P3_3 = 0x21;
    LPC_IOCON->P3_4 = 0x21;
    LPC_IOCON->P3_5 = 0x21;
    LPC_IOCON->P3_6 = 0x21;
    LPC_IOCON->P3_7 = 0x21;
    LPC_IOCON->P3_8 = 0x21;
    LPC_IOCON->P3_9 = 0x21;
    LPC_IOCON->P3_10 = 0x21;
    LPC_IOCON->P3_11 = 0x21;
    LPC_IOCON->P3_12 = 0x21;
    LPC_IOCON->P3_13 = 0x21;
    LPC_IOCON->P3_14 = 0x21;
    LPC_IOCON->P3_15 = 0x21;
//    LPC_IOCON->P3_16 = 0x21;//
//    LPC_IOCON->P3_17 = 0x21;
//    LPC_IOCON->P3_18 = 0x21;
//    LPC_IOCON->P3_19 = 0x21;
//    LPC_IOCON->P3_20 = 0x21;
//    LPC_IOCON->P3_21 = 0x21;
//    LPC_IOCON->P3_22 = 0x21;
//    LPC_IOCON->P3_23 = 0x21;
//    LPC_IOCON->P3_24 = 0x21;
//    LPC_IOCON->P3_25 = 0x21;
//    LPC_IOCON->P3_26 = 0x21;
//    LPC_IOCON->P3_27 = 0x21;
//    LPC_IOCON->P3_28 = 0x21;
//    LPC_IOCON->P3_29 = 0x21;
//    LPC_IOCON->P3_30 = 0x21;
//    LPC_IOCON->P3_31 = 0x21;//
    LPC_IOCON->P4_0 = 0x21;
    LPC_IOCON->P4_1 = 0x21;
    LPC_IOCON->P4_2 = 0x21;
    LPC_IOCON->P4_3 = 0x21;
    LPC_IOCON->P4_4 = 0x21;
    LPC_IOCON->P4_5 = 0x21;
    LPC_IOCON->P4_6 = 0x21;
    LPC_IOCON->P4_7 = 0x21;
    LPC_IOCON->P4_8 = 0x21;
    LPC_IOCON->P4_9 = 0x21;
    LPC_IOCON->P4_10 = 0x21;
    LPC_IOCON->P4_11 = 0x21;
    LPC_IOCON->P4_12 = 0x21;
    LPC_IOCON->P4_13 = 0x21;
    LPC_IOCON->P4_14 = 0x21;
    LPC_IOCON->P4_25 = 0x21;
    /* Power up SDRAM controller */
    LPC_SC->PCONP   	|= 0x00000800;
    /*Init SDRAM controller*/
    /*Set data read delay*/
    LPC_SC->EMCDLYCTL |= (0x0a << 0);
    LPC_SC->EMCDLYCTL |= (0x0a << 8);
    LPC_SC->EMCDLYCTL |= (0x0a << 16);

    LPC_EMC->Control           = 1;
    LPC_EMC->DynamicReadConfig = 1;
    LPC_EMC->DynamicRasCas0    = (SDRAM_RAS_LATENCY << 0) | (SDRAM_CAS_LATENCY << 8);
    LPC_EMC->DynamicRP         = P2C(SDRAM_TRP);
    LPC_EMC->DynamicRAS        = P2C(SDRAM_TRAS);
    LPC_EMC->DynamicSREX       = P2C(SDRAM_TXSR);
    LPC_EMC->DynamicAPR        = P2C(SDRAM_TAPR);
    LPC_EMC->DynamicDAL        = SDRAM_TDAL + P2C(SDRAM_TRP) + 1;
    LPC_EMC->DynamicWR         = SDRAM_TWR - 1;
    LPC_EMC->DynamicRC         = P2C(SDRAM_TRC);
    LPC_EMC->DynamicRFC        = P2C(SDRAM_TRFC);
    LPC_EMC->DynamicXSR        = P2C(SDRAM_TXSR);
    LPC_EMC->DynamicRRD        = P2C(SDRAM_TRRD);
    LPC_EMC->DynamicMRD        = SDRAM_TMRD - 1;

    // 13 row, 9 - col
#ifdef ROW_BANK_COLUMN_MAP
    LPC_EMC->DynamicConfig0 = (0 << 14) | (0 << 12) | (0x03 << 9) | (0x01 << 7);
#else
    LPC_EMC->DynamicConfig0 = (0 << 14) | (1 << 12) | (0x03 << 9) | (0x01 << 7);
#endif

    // JEDEC General SDRAM Initialization Sequence
    // DELAY to allow power and clocks to stabilize ~100 us
    // NOP
    LPC_EMC->DynamicControl = 0x0183;
    stimer_wait_us(200);
//    for(i = 200*30; i; i--);
    // PALL
    LPC_EMC->DynamicControl = 0x0103;
    LPC_EMC->DynamicRefresh = 1;
    stimer_wait_us(10);
//    for(i = 256; i; --i); // > 128 clk
    LPC_EMC->DynamicRefresh = (P2C(SDRAM_REFRESH)) >> 4;
    // COMM
    LPC_EMC->DynamicControl    = 0x00000083; /* Issue MODE command */

#define SDRAM_COLUMN_BITS       9
#define SDRAM_ROW_BITS          13 
#define SDRAM_BANK_BITS         2
#define SDRAM_DATA_BITS         16
#define SDRAM_MODE_REG_VALUE    ((SDRAM_CAS_LATENCY << 4) | 0x03)

#ifdef ROW_BANK_COLUMN_MAP
    Dummy = *((volatile uint32_t *)(SDRAM_BASE_ADDR |
            (SDRAM_MODE_REG_VALUE << (SDRAM_COLUMN_BITS + SDRAM_BANK_BITS + SDRAM_DATA_BITS / 16))));
#else
    Dummy = *((volatile uint32_t *)(SDRAM_BASE_ADDR |
            (SDRAM_MODE_REG_VALUE << (SDRAM_COLUMN_BITS + SDRAM_DATA_BITS / 16))));
#endif

    (void)Dummy;
    // NORM
    LPC_EMC->DynamicControl = 0x0000;

    /* enable buffer */
    LPC_EMC->DynamicConfig0 |= (1 << 19);
//    LPC_EMC->DynamicConfig0 |=((1<<19)|(0<<20));
//    for(i = 100000; i; i--);
    stimer_wait_us(200);
}

/*
 * RETURN
 *     1 on success, 0 on error
 */
int  __attribute__((optimize("O0"))) sdram_test(int type)
{
    int ntest;
    uint32 tm;
    volatile uint16 *p16;
    volatile uint32 *p32;
    uint16 val;
    uint32 bank, row, column, n, i;
    uint32 mask;

    for (ntest = 0; ntest < 2; ntest++)
    {
        dprint("<SDRAM test #>1d*:n", ntest);

#ifndef ROW_BANK_COLUMN_MAP
        if (type == SDRAM_TEST_SHORT)
        {
            /*
             * Test each line of address (each row, each bank)
             */
            dprint("ts", "short test... ");
            stimer_settime_us(&tm);

            p16 = (volatile uint16*)SDRAM_BASE_ADDR;
            for (bank = 0; bank < (1 << SDRAM_BANK_BITS); bank++)
            {
                for (row = 0; row < (1 << SDRAM_ROW_BITS); row++)
                {
                    *p16 = row | (bank << 14);
                    p16 += (1 << SDRAM_COLUMN_BITS);
                }
            }

            p16 = (volatile uint16*)SDRAM_BASE_ADDR;
            for (bank = 0; bank < (1 << SDRAM_BANK_BITS); bank++)
            {
                for (row = 0; row < (1 << SDRAM_ROW_BITS); row++)
                {
                    if (*p16 != (row | (bank << 14)))
                    {
                        dprint("sn", "FAILED");
                        dprint("tt<row  >4xn", row);
                        dprint("tt<bank >4xn", bank);
                        return 0;
                    }
                    p16 += (1 << SDRAM_COLUMN_BITS);
                }
            }

            tm = stimer_deltatime_us(tm);
            dprint("<done (>4d*.4d< ms)>n", tm / 1000, tm % 1000);
        } else if (type == SDRAM_TEST_FULL) {
            /*
             * Clean-up memory
             */
            dprint("ts", "clean-up memory         ");
            stimer_settime_us(&tm);
            p32 = (volatile uint32*)SDRAM_BASE_ADDR;
            for (i = 0; i < (SDRAM_SIZE / 4); i++)
            {
                *p32 = 0;
                if (*p32)
                {
                    dprint("sn", "FAILED");
                    dprint("tt<addr  >4xn", p32);
                }
                p32++;
            }
            tm = stimer_deltatime_us(tm);
            dprint("<done (>4d*.4d< ms)>n", tm / 1000, tm % 1000);

            dprint("ts", "running bit             ");
            stimer_settime_us(&tm);
            p16 = (volatile uint16*)SDRAM_BASE_ADDR;
            for (mask = 1; mask < 0x10000; mask <<= 1)
            {
                *p16 = mask & 0xffff;
                if (*p16 != mask)
                {
                    dprint("sn", "FAILED");
                    dprint("tt<mask  >4xn", mask);
                }
            }
            tm = stimer_deltatime_us(tm);
            dprint("<done (>4d*.4d< ms)>n", tm / 1000, tm % 1000);

            /*
             * Sequential write all memory locations by 16-bit
             */
            dprint("ts", "16-bit write            ");
            stimer_settime_us(&tm);
            n = 0;
            p16 = (volatile uint16*)SDRAM_BASE_ADDR;
            for (bank = 0; bank < (1 << SDRAM_BANK_BITS); bank++)
            {
                for (row = 0; row < (1 << SDRAM_ROW_BITS); row++)
                {
                    for (column = 0; column < (1 << SDRAM_COLUMN_BITS); column++)
                    {
                        *p16 = n & 0xffff;
                        p16++;
                        n++;
                    }
                    n++;
                }
            }
            n = 0;

            p16 = (volatile uint16*)SDRAM_BASE_ADDR;
            for (bank = 0; bank < (1 << SDRAM_BANK_BITS); bank++)
            {
                for (row = 0; row < (1 << SDRAM_ROW_BITS); row++)
                {
                    for (column = 0; column < (1 << SDRAM_COLUMN_BITS); column++)
                    {
                        val = *p16;
                        if (val != (n & 0xffff))
                        {
                            dprint("sn", "FAILED");
                            dprint("tt<addr   >4xn", p16);
                            dprint("tt<col    >4xn", column);
                            dprint("tt<row    >4xn", row);
                            dprint("tt<bank   >4xn", bank);
                            dprint("tt<write  >2xn", n);
                            dprint("tt<read   >2xn", val);
                            return 0;
                        }
                        p16++;
                        n++;
                    }
                    n++;
                }
            }
            tm = stimer_deltatime_us(tm);
            dprint("<done (>4d*.4d< ms)>n", tm / 1000, tm % 1000);
        }
#endif
    }

    return 1;
}


