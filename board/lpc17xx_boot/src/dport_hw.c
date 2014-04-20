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
#include <LPC177x_8x_bits.h>
#include <clk_cfg.h>
#include "dport_hw.h"

static void dport_hw_uart_setup();
static void dport_hw_iap_setup();

/*
 *
 */
void dport_hw_init()
{
    dport_hw_uart_setup();
    dport_hw_iap_setup();
//    dport_hw_timer_setup();
}

/*************************************************
 * UART functions
 *************************************************/
#define UART_BAUD_38400_20_MHZ      ((0 << 0) | (19 << 8) | (5 << 16) | (7 << 24))
#define UART_BAUD_38400_24_MHZ      ((0 << 0) | (23 << 8) | (7 << 16) | (10 << 24))
#define UART_BAUD_115200_20_MHZ     ((0 << 0) | (8 << 8) | (5 << 16) | (14 << 24))
#define UART_BAUD_115200_24_MHZ     ((0 << 0) | (7 << 8) | (6 << 16) | (7 << 24))
#define UART_BAUD_115200_36_MHZ     ((0 << 0) | (13 << 8) | (1 << 16) | ( 2 << 24))

#if   CLK_PCLK == (36 * 1000000)                 
    #define UART_BAUD_115200    UART_BAUD_115200_36_MHZ
#elif CLK_PCLK   == (24 * 1000000)
    #define UART_BAUD_38400     UART_BAUD_38400_24_MHZ
    #define UART_BAUD_115200    UART_BAUD_115200_24_MHZ
#elif CLK_PCLK == (20 * 1000000)                 
    #define UART_BAUD_38400     UART_BAUD_38400_20_MHZ
    #define UART_BAUD_115200    UART_BAUD_115200_20_MHZ
#else
    #error Unknown PCLK frequency
#endif


//#if CLK_PCLK == (24 * 1000000)
//    #define UART_BAUD_38400     UART_BAUD_38400_24_MHZ
//    #define UART_BAUD_115200    UART_BAUD_115200_24_MHZ
//#elif CLK_PCLK == (20 * 1000000)                 
//    #define UART_BAUD_38400     UART_BAUD_38400_20_MHZ
//    #define UART_BAUD_115200    UART_BAUD_115200_20_MHZ
//#else
//    #error Unknown PCLK frequency
//#endif

#define UART_BAUD    UART_BAUD_115200
/*
 *
 */
static void dport_hw_uart_setup()
{
    /* setup UART0 */
    {
        LPC_SC->PCONP |= PCONP_PCUART0;

        /* P0.2 (TXD0), P0.3 (RXD0) */
        LPC_IOCON->P0_2 &= ~LPC_IOCON_FUNC_MASK;
        LPC_IOCON->P0_2 |= LPC_IOCON_FUNC(1);
        LPC_IOCON->P0_3 &= ~LPC_IOCON_FUNC_MASK;
        LPC_IOCON->P0_3 |= LPC_IOCON_FUNC(1);

        LPC_UART0->LCR = UART_LCR_DLAB;

        /*
         * CCLK/4 = 22.11834 MHz
         *
         *                         PCLK 
         * BAUD =  ----------------------------------------
         *         (16 * (256 * DLM + DLL) * (1 + DIV/MUL))
         *
         *
         */

        /*
         * bits of speed argument:
         *
         * 0..7   bits  DLM
         * 8..15  bits  DLL
         * 16..23 bits  DIV
         * 24..31 bits  MUL
         */
        {
            uint8 dlm;
            uint8 dll;
            uint8 div;
            uint8 mul;

            dlm = (UART_BAUD >> 0) & 0xff;
            dll = (UART_BAUD >> 8) & 0xff;
            div = (UART_BAUD >> 16) & 0xff;
            mul = (UART_BAUD >> 24) & 0xff;

            LPC_UART0->DLM = dlm;
            LPC_UART0->DLL = dll;
            LPC_UART0->FDR = (div << 0) | (mul << 4);
            LPC_UART0->LCR = (3 << 0);
        }

        /* Reset RX, TX FIFO's, activate FIFO's, 0 characters for CTI interrupt */
        LPC_UART0->FCR = (1 << 2) | (1 << 1) | (1 << 0) | (0 << 6);

        /* Enable receive interrupt */
        LPC_UART0->IER = UART_IER_RBR;
    }

    /* enable UART interrupt, set priority to maximum */
    NVIC_EnableIRQ(UART0_IRQn);
    NVIC_SetPriority(UART0_IRQn, 0);
}

/*
 *
 */
int dport_hw_getchar(char *c)
{
    uint32 intid;

    intid = (LPC_UART0->IIR >> 1) & 0x07;

    if (intid == UART_INTID_RLS || intid == UART_INTID_RDA || intid == UART_INTID_CTI)
    {
        if (LPC_UART0->LSR & UART_LSR_RDR)
        {
            *c = LPC_UART0->RBR;
//            dport_hw_putchar((*c >> 4)   >= 10 ? ((*c >> 4)   + 'a' - 10): (*c >> 4)   + '0');
//            dport_hw_putchar((*c & 0x0f) >= 10 ? ((*c & 0x0f) + 'a' - 10): (*c & 0x0f) + '0');
//            dport_hw_putchar('\r');
//            dport_hw_putchar('\n');
            return 1;
        }
    }

    return 0;
}

/*
 *
 */
int dport_hw_putchar(char c) 
{ 
    while (!(LPC_UART0->LSR & UART_LSR_THRE))
        ;
    LPC_UART0->THR = c;

    return c;
}

/*************************************************
 * Flash write functions (IAP)
 *************************************************/

struct flash_sector_t {
    uint32 size; /* sector size in bytes */
    uint32 start; /* sector start address */
};

#define SECTOR_COUNT 30
const struct flash_sector_t sectors[SECTOR_COUNT] = {
    { (4 * 1024),  0x00000000},  /* 0   00  0x00000000 - 0x00000FFF */
    { (4 * 1024),  0x00001000},  /* 1   01  0x00001000 - 0x00001FFF */
    { (4 * 1024),  0x00002000},  /* 2   02  0x00002000 - 0x00002FFF */
    { (4 * 1024),  0x00003000},  /* 3   03  0x00003000 - 0x00003FFF */
    { (4 * 1024),  0x00004000},  /* 4   04  0x00004000 - 0x00004FFF */
    { (4 * 1024),  0x00005000},  /* 5   05  0x00005000 - 0x00005FFF */
    { (4 * 1024),  0x00006000},  /* 6   06  0x00006000 - 0x00006FFF */
    { (4 * 1024),  0x00007000},  /* 7   07  0x00007000 - 0x00007FFF */
    { (4 * 1024),  0x00008000},  /* 8   08  0x00008000 - 0x00008FFF */
    { (4 * 1024),  0x00009000},  /* 9   09  0x00009000 - 0x00009FFF */
    { (4 * 1024),  0x0000a000},  /* 10  0a  0x0000A000 - 0x0000AFFF */
    { (4 * 1024),  0x0000b000},  /* 11  0b  0x0000B000 - 0x0000BFFF */
    { (4 * 1024),  0x0000c000},  /* 12  0c  0x0000C000 - 0x0000CFFF */
    { (4 * 1024),  0x0000d000},  /* 13  0d  0x0000D000 - 0x0000DFFF */
    { (4 * 1024),  0x0000e000},  /* 14  0e  0x0000E000 - 0x0000EFFF */
    { (4 * 1024),  0x0000f000},  /* 15  0f  0x0000F000 - 0x0000FFFF */
    { (32 * 1024), 0x00010000},  /* 16  10  0x00010000 - 0x00017FFF */
    { (32 * 1024), 0x00018000},  /* 17  11  0x00018000 - 0x0001FFFF */
    { (32 * 1024), 0x00020000},  /* 18  12  0x00020000 - 0x00027FFF */
    { (32 * 1024), 0x00028000},  /* 19  13  0x00028000 - 0x0002FFFF */
    { (32 * 1024), 0x00030000},  /* 20  14  0x00030000 - 0x00037FFF */
    { (32 * 1024), 0x00038000},  /* 21  15  0x00038000 - 0x0003FFFF */
    { (32 * 1024), 0x00040000},  /* 22  16  0x00040000 - 0x00047FFF */
    { (32 * 1024), 0x00048000},  /* 23  17  0x00048000 - 0x0004FFFF */
    { (32 * 1024), 0x00050000},  /* 24  18  0x00050000 - 0x00057FFF */
    { (32 * 1024), 0x00058000},  /* 25  19  0x00058000 - 0x0005FFFF */
    { (32 * 1024), 0x00060000},  /* 26  1a  0x00060000 - 0x00067FFF */
    { (32 * 1024), 0x00068000},  /* 27  1b  0x00068000 - 0x0006FFFF */
    { (32 * 1024), 0x00070000},  /* 28  1c  0x00070000 - 0x00077FFF */
    { (32 * 1024), 0x00078000},  /* 29  1d  0x00078000 - 0x0007FFFF */
};


#define IAP_LOCATION   0x1fff1ff1

#define IAP_CMD_PREPARE_SECTORS_FOR_WRITE 50
#define IAP_CMD_COPY_RAM_TO_FLASH         51
#define IAP_CMD_ERASE_SECTORS             52
#define IAP_CMD_BLANK_CHECK_SECTORS       53
#define IAP_CMD_READ_PART_ID              54

#define IAP_RCODE_CMD_SUCCESS                             0
#define IAP_RCODE_INVALID_COMMAND                         1
#define IAP_RCODE_SRC_ADDR_ERROR                          2
#define IAP_RCODE_DST_ADDR_ERROR                          3
#define IAP_RCODE_SRC_ADDR_NOT_MAPPED                     4
#define IAP_RCODE_DST_ADDR_NOT_MAPPED                     5
#define IAP_RCODE_COUNT_ERROR                             6
#define IAP_RCODE_INVALID_SECTOR                          7
#define IAP_RCODE_SECTOR_NOT_BLANK                        8
#define IAP_RCODE_SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION 9
#define IAP_RCODE_COMPARE_ERROR                           10
#define IAP_RCODE_BUSY                                    11



static unsigned int command[5];
static unsigned int result[5];

uint8 flashbuf[FLASHBUF_SIZE];

typedef void (*IAP) (unsigned int [], unsigned int []);

static IAP dport_hw_iap_entry;

static uint32 dport_hw_iap_prepare(uint32 start, uint32 end);
static uint32 dport_hw_iap_erase(uint32 start, uint32 end);
static uint32 dport_hw_iap_ram2flash(uint8 *data, uint32 len, uint32 offs);

static uint32 addr2sector(uint32 addr);
static uint32 addrisstart(uint32 addr);

/*
 *
 */
static void dport_hw_iap_setup()
{
    dport_hw_iap_entry = (IAP)IAP_LOCATION;

    command[0] = IAP_CMD_READ_PART_ID;
    dport_hw_iap_entry(command, result);
}

/*
 * wrapper for completely write data to flash
 *
 * NOTE interrupts should be disabled when using internal IAP functions
 *
 * ARGS
 *     data    pointer to data
 *     len     length of data
 *     offs    offset of flash
 *
 * RETURN
 *     zero on success or IAP return code otherwise
 *
 */
uint32 dport_hw_iap_write(uint8 *data, uint32 len, uint32 offs)
{
    uint32 sector;
    uint32 ret;

    sector = addr2sector(offs);

    ret = 0;
    if (addrisstart(offs))
    {
        ret = dport_hw_iap_prepare(sector, sector);
        if (ret != IAP_RCODE_CMD_SUCCESS)
            goto out;

        ret = dport_hw_iap_erase(sector, sector);
        if (ret != IAP_RCODE_CMD_SUCCESS)
            goto out;
    }

    ret = dport_hw_iap_prepare(sector, sector);
    if (ret != IAP_RCODE_CMD_SUCCESS)
        goto out;

    ret = dport_hw_iap_ram2flash(data, len, offs);
    if (ret != IAP_RCODE_CMD_SUCCESS)
        goto out;

out:
    return ret;
}

/*
 * ARGS
 *    start  start sector
 *    end    end   sector
 */
static uint32 dport_hw_iap_prepare(uint32 start, uint32 end)
{
    command[0] = IAP_CMD_PREPARE_SECTORS_FOR_WRITE;
    command[1] = start;
    command[2] = end;
    dport_hw_iap_entry(command, result);
    return result[0];
}

/*
 * ARGS
 *     start  start sector
 *     end    end sector
 *
 */
static uint32 dport_hw_iap_erase(uint32 start, uint32 end)
{
    command[0] = IAP_CMD_ERASE_SECTORS;
    command[1] = start;
    command[2] = end;
    command[3] = CLK_CCLK/1000;
    dport_hw_iap_entry(command, result);
    return result[0];
}

/*
 * ARGS
 *     start  start sector
 *     end    end sector
 *
 */
static uint32 dport_hw_iap_ram2flash(uint8 *data, uint32 len, uint32 offs)
{
    command[0] = IAP_CMD_COPY_RAM_TO_FLASH;
    command[1] = offs;
    command[2] = (unsigned int)data;
    command[3] = len;
    command[4] = CLK_CCLK/1000;
    dport_hw_iap_entry(command, result);
    return result[0];
}

/*
 * return sector number from appropriate address
 */
static uint32 addr2sector(uint32 addr)
{
    uint32 i;
    uint32 end;

    for (i = 0; i < SECTOR_COUNT; i++)
    {
        end = sectors[i].start + sectors[i].size;
        if (addr >= sectors[i].start && addr < end)
            return i;
    }

    return (SECTOR_COUNT - 1); /* NOTREACHED */
}

/*
 * check whether address is start address of sector
 *
 * RETURN
 *    1   if address is start of sector
 *    0   otherwise
 */
static uint32 addrisstart(uint32 addr)
{
    uint32 i;

    for (i = 0; i < SECTOR_COUNT; i++)
    {
        if (addr == sectors[i].start)
            return 1;
    }

    return 0;
}

