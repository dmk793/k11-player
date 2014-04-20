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
 * SDRAM Chip: K4S561632J
 */
#ifndef SDRAM_H
#define SDRAM_H

#include <clk_cfg.h>
#include <types.h>

#define SDRAM_BASE_ADDR     0xA0000000
#define SDRAM_SIZE          0x2000000

//#if   CLK_EMC == (120 * 1000000)
//    #define SDRAM_PERIOD          8.33  /* 120 MHz  */
//#elif CLK_EMC == (100 * 1000000)                 
//    #define SDRAM_PERIOD          10.0  /* 96 MHz    */
//#elif CLK_EMC == (96 * 1000000)                 
//    #define SDRAM_PERIOD          10.4  /* 96 MHz    */
//#elif CLK_EMC == (72 * 1000000)                   
//    #define SDRAM_PERIOD          13.8  /* 72 MHz    */
//#elif CLK_EMC == (60 * 1000000)                   
//    #define SDRAM_PERIOD          16.67 /* 60 MHz   */
//#elif CLK_EMC == (57 * 1000000)
//    #define SDRAM_PERIOD          17.4  /* 57.6 MHz  */
//#elif CLK_EMC == (48 * 1000000)
//    #define SDRAM_PERIOD          20.8  /* 48 MHz    */
//#elif CLK_EMC == (36 * 1000000)
//    #define SDRAM_PERIOD          27.8  /* 36 MHz    */
//#elif CLK_EMC == (24 * 1000000)
//    #define SDRAM_PERIOD          41.7  /* 24 MHz    */
//#elif CLK_EMC == (12 * 1000000)
//    #define SDRAM_PERIOD          83.3  /* 12 MHz    */
//#else
//    #error Frequency not defined                    
//#endif

//#define P2C(Period)           (((Period<SDRAM_PERIOD)?0:(uint32_t)((double)Period/SDRAM_PERIOD))+1)
//#define P2C(Period)           (((Period < SDRAM_PERIOD) ? 1 : (uint32_t)((double)Period/SDRAM_PERIOD)) + 1)
//#define P2C(Period)           ((uint32_t)((double)Period/SDRAM_PERIOD))

#define tCLK_ns ((double)1000000000.0 / CLK_EMC)         // CCLK period in ns      
#define P2C(ns) ((uint32_t)( (double)(ns) / tCLK_ns ))   // convert ns to CCLKs

//#define SDRAM_REFRESH         7813
//#define SDRAM_TRP             20
//#define SDRAM_TRAS            45
//#define SDRAM_TAPR            1
//#define SDRAM_TDAL            3
//#define SDRAM_TWR             3
//#define SDRAM_TRC             65
//#define SDRAM_TRFC            66
//#define SDRAM_TXSR            67
//#define SDRAM_TRRD            15
//#define SDRAM_TMRD            3

#define SDRAM_RAS_LATENCY    2
#define SDRAM_CAS_LATENCY    2

/* 8192 cycles thru 64 ms ~ 7812 ns */
#define SDRAM_REFRESH         7812   /* ns */
#define SDRAM_TRP             20     /* ns */
#define SDRAM_TRAS            45     /* ns */
#define SDRAM_TAPR            SDRAM_TRP      /* NOTE not tAPR value, using tRP value */
#define SDRAM_TDAL            2      /* CLK */
#define SDRAM_TWR             2      /* tWR, tDPL, tRWL or tRDL: CLK */
#define SDRAM_TRC             65     /* ns */
#define SDRAM_TRFC            66     /* ns */
#define SDRAM_TXSR            67     /* ns */
#define SDRAM_TRRD            15     /* ns */
#define SDRAM_TMRD            3

void sdram_init();
#define SDRAM_TEST_SHORT   0
#define SDRAM_TEST_FULL    1
int sdram_test(int type);

#endif

