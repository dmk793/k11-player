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
#include <os.h>
#include <debug.h>
#include <gpio.h>
#include "sdcard.h"
#include "sdcard_hw.h"
#include "../dma.h"
#include "../irqp.h"

#define DEBUG_SDCARD_HW

#ifdef DEBUG_SDCARD_HW
    #define DPRINT(fmt, ...)  dprint(fmt, __VA_ARGS__)
#else
    #define DPRINT(fmt, ...)
#endif

static int waitfor(uint32 mask);

static struct gpio_t detect = {LPC_GPIO1, (1 << 13)};
#define EVENT_MASK_IRQ    (1 << 0)
static BASE_TYPE event;

/* 
 * SD_LO_CLK should not exceed 400 kHz.
 * SD_HI_CLK should not exceed 25 MHz.
 */
#define SD_LO_CLK_MAX    (200 * 1000)
#define SD_HI_CLK_MAX    (12 * 1000 * 1000)

#define SD_LO_CLK_DIV   ((CLK_PCLK / (SD_LO_CLK_MAX / 1000 * 2) / 1000) - 1)
#define SD_HI_CLK_DIV   ((CLK_PCLK / (SD_HI_CLK_MAX / 1000 * 2) / 1000) - 1)

#define SD_LO_CLK   (CLK_PCLK * 10 / (2 * (SD_LO_CLK_DIV + 1)) / 10)
#define SD_HI_CLK   (CLK_PCLK * 10 / (2 * (SD_HI_CLK_DIV + 1)) / 10)
//#define SD_HI_CLK    CLK_PCLK /* NOTE bypass is used */

#if (SD_LO_CLK > (400 * 1000)) || (SD_HI_CLK > (25 * 1000 * 1000))
   #error "Check clock config for SD card"
#endif

/*
 *
 */
void sdcard_hw_init()
{
    LPC_SC->PCONP |= PCONP_PCSDC;

    LPC_MCI->POWER = 0;
    /*
     * configure pins
     *
     * Function   Port    MCU Pin   Net Name    Connector pinout
     * -----------------------------------------------------------
     * SD_DAT0    P1[6]   PIN171    C_D0        7    DAT0
     * SD_DAT1    P1[7]   PIN153    C_D1        8    DAT1
     * SD_DAT2    P1[11]  PIN163    C_D2        1    DAT2
     * SD_DAT3    P1[12]  PIN157    C_D3        2    CD/DAT3
     * SD_CMD     P1[3]   PIN177    C_CMD       3    Command Line
     * SD_CLK     P1[2]   PIN185    C_CLK       5    Clock
     *                              X_DET       9    Card_detect
     *            P1[13]            MP147      10    Card_detect1
     *                                          4    Vdd
     *                                          6    GND
     *
     * SD_PWR     P0[21]            unused
     */
    LPC_IOCON_FUNC_SET(P1_6, LPC_IOCON_P1_6_SD_DAT0 );
    LPC_IOCON_FUNC_SET(P1_7, LPC_IOCON_P1_7_SD_DAT1 );
    LPC_IOCON_FUNC_SET(P1_11, LPC_IOCON_P1_11_SD_DAT2);
    LPC_IOCON_FUNC_SET(P1_12, LPC_IOCON_P1_12_SD_DAT3);
    LPC_IOCON_FUNC_SET(P1_3, LPC_IOCON_P1_3_SD_CMD  );
    LPC_IOCON_FUNC_SET(P1_2, LPC_IOCON_P1_2_SD_CLK  );

    gpio_setdir(&detect, GPIO_DIR_INPUT);

    /* XXX */
//    LPC_MCI->POWER = SD_PWR_CTRL_POWER_ON | SD_PWR_OPENDRAIN;
    LPC_MCI->POWER = SD_PWR_CTRL_POWER_ON;
    {
        int i;
        for (i = 0; i < 10000; i++)
            asm volatile("nop");
    }


    NVIC_SetPriority(MCI_IRQn, MCI_IRQP);

    dprint("sg4dsn", "SD card, PCLK = ", 5, CLK_PCLK / 1000, " kHz");
    dprint("sg4dsn", "SD card, LCLK = ", 5, SD_LO_CLK / 1000, " kHz");
    dprint("sg4dsn", "SD card, HCLK = ", 5, SD_HI_CLK / 1000, " kHz");
}

/*
 *
 */
void sdcard_hw_set_lo_clk()
{
    LPC_MCI->CLOCK = 
        SD_CLOCK_CLKDIV(SD_LO_CLK_DIV) |
        SD_CLOCK_ENABLE |
        SD_CLOCK_PWRSAVE |
        SD_CLOCK_WIDEBUS;

}

/*
 *
 */
void sdcard_hw_set_hi_clk()
{
#if (SD_HI_CLK != CLK_PCLK)
    LPC_MCI->CLOCK = 
        SD_CLOCK_ENABLE |
        SD_CLOCK_CLKDIV(SD_HI_CLK_DIV) |
        SD_CLOCK_PWRSAVE |
        SD_CLOCK_WIDEBUS;
#else
    LPC_MCI->CLOCK = 
        SD_CLOCK_ENABLE |
        SD_CLOCK_BYPASS |
        SD_CLOCK_PWRSAVE |
        SD_CLOCK_WIDEBUS;
#endif
}

/*
 * ARGS
 *     cmd    command index
 *     arg    argument
 *     flags  XXX
 *             SD_CMDFLAG_WAITRESPONSE
 *             SD_CMDFLAG_LONGRESPONSE
 *
 * RETURN 
 *     zero on success, errorcode otherwise
 *
 */
int sdcard_hw_send_cmd(uint8 cmd, uint32 flags, union sd_argument_t *arg, union sd_response_t *resp)
{
    if (LPC_MCI->STATUS & SD_STATUS_CMDACTIVE)
    {
        DEBUG_WMSG("command still active");
    }
    LPC_MCI->COMMAND = 0; /* NOTE */

    /* clear status bits */
    LPC_MCI->CLEAR =
        SD_STATUS_CMDCRCFAIL |
        SD_STATUS_CMDTIMEOUT |
        SD_STATUS_CMDRESPEND |
        SD_STATUS_CMDSENT;

    if (arg)
        LPC_MCI->ARGUMENT = arg->value;
    else
        LPC_MCI->ARGUMENT = 0;
    LPC_MCI->COMMAND = SD_COMMAND_ENABLE | flags | SD_COMMAND_CMDINDEX(cmd);

    if (flags & SD_COMMAND_RESPONSE)
    {
        if (!waitfor(SD_STATUS_CMDCRCFAIL | SD_STATUS_CMDTIMEOUT | SD_STATUS_CMDRESPEND))
            return (1 << 31); /* XXX */

        if (LPC_MCI->STATUS & SD_STATUS_CMDCRCFAIL)
        {
            /*
             * NOTE
             * ACMD41 always returned with CRC failed (CRC field set as 0b1111111)
             * but should have valid response.
             */
            resp->sresp = LPC_MCI->RESP0;
            return SD_STATUS_CMDCRCFAIL;
        }
        if (LPC_MCI->STATUS & SD_STATUS_CMDTIMEOUT)
            return SD_STATUS_CMDTIMEOUT;
    } else {
        if (!waitfor(SD_STATUS_CMDSENT))
            return (1 << 31); /* XXX */
    }

    if (resp)
    {
        if (flags & SD_COMMAND_LONG)
        {
            /* NOTE move data in invert order */
            resp->lresp[0] = LPC_MCI->RESP3;
            resp->lresp[1] = LPC_MCI->RESP2;
            resp->lresp[2] = LPC_MCI->RESP1;
            resp->lresp[3] = LPC_MCI->RESP0;
        } else {
            resp->sresp = LPC_MCI->RESP0;
        }
    }

    return 0;
}

/*
 *
 * NOTE
 * for SDHC and SDHC host should use 100ms timeout (minimum)
 * for single and multiple read operation rather than using TAAC and SAAC
 *
 * RETURN
 *     1 on success, 0 otherwise
 */
#define DATA_TIMER_VALUE    ((100 + 20) * (SD_HI_CLK / 1000)) /* in unit of SD_CLK */
int sdcard_hw_read_block(uint8 *data)
{
    int ret;

    ret = 1;

    LPC_MCI->CLEAR    = SD_CLEAR_MASK;
    LPC_MCI->DATATMR  = DATA_TIMER_VALUE;
    LPC_MCI->DATALEN  = cardstate.blocklen;
#if 1

    if (!dma_sd_read(data))
    {
        ret = 0;
        goto out;
    }

    LPC_MCI->DATACTRL = 
        SD_DATACTRL_ENABLE |
        SD_DATACTRL_DMAENABLE |
        SD_DATACTRL_DIRECTION_READ | 
        SD_DATACTRL_BLOCKSIZE(9); /* XXX fixed to 512 bytes */

    if (!waitfor(SD_STATUS_DATAEND | SD_STATUS_RXOVERRUN | SD_STATUS_DATATIMEOUT))
    {
        dma_sd_cancel();
        ret = 0;
        goto out;
    }

    if (LPC_MCI->STATUS & (SD_STATUS_RXOVERRUN | SD_STATUS_DATATIMEOUT))
    {
        if (LPC_MCI->STATUS & SD_STATUS_RXOVERRUN)
            DEBUG_EMSG("RXOVERRUN");
        if (LPC_MCI->STATUS & SD_STATUS_DATATIMEOUT)
            DEBUG_EMSG("DATATIMEOUT");

        dma_sd_cancel();
        ret = 0;
        goto out;
    }

#define SD_CARD_DMA_TIMEOUT   200
    if (!dma_wait_chan(DMA_CHMASK_SD, SD_CARD_DMA_TIMEOUT))
        ret = 0;
#else
    /* XXX code below not work, check closely */

    LPC_MCI->DATACTRL = 
        SD_DATACTRL_ENABLE |
        SD_DATACTRL_DIRECTION_READ | 
        SD_DATACTRL_BLOCKSIZE(9); /* XXX fixed to 512 bytes */

    waitfor(SD_STATUS_RXACTIVE);

    while (LPC_MCI->FIFOCNT)
    {
        dprint("s4xn", "FIFOCNT  = ", LPC_MCI->FIFOCNT);

        uint32 val;
        waitfor(SD_STATUS_RXDATAAVLBL);

        val = LPC_MCI->FIFO;
        *data++ = (val >>  0) & 0xff;
        *data++ = (val >>  8) & 0xff;
        *data++ = (val >> 16) & 0xff;
        *data++ = (val >> 24) & 0xff;
    }
#endif

out:
    LPC_MCI->DATACTRL = 0; /* NOTE */
    return ret;
}


/*
 * wait for status
 *
 * RETURN
 *     1 on success, 0 otherwise
 *
 */
static int waitfor(uint32 mask)
{
    NVIC_ClearPendingIRQ(MCI_IRQn);
    LPC_MCI->MASK0 = mask;
    NVIC_EnableIRQ(MCI_IRQn);

#define WAITFOR_TIMEOUT    (100 + 20) /* ms */

    /* NOTE wait with timeout */
    if (os_event_wait(&event, EVENT_MASK_IRQ, 
            OS_FLAG_CLEAR, OS_MS2TICK(WAITFOR_TIMEOUT)) != OS_ERR_NONE)
    {
        DEBUG_EMSG("timeout");
    }
    
    NVIC_DisableIRQ(MCI_IRQn);

    LPC_MCI->MASK0 = 0;
    if ((LPC_MCI->STATUS & mask) == 0)
    {
        DEBUG_EMSGF("FAILED", "<wait mask >4xn", mask);
        return 0;
    }

    return 1;
}

/*
 *
 */
void SDCard_Handler(void)
{
    NVIC_DisableIRQ(MCI_IRQn);
    os_event_raise(&event, EVENT_MASK_IRQ);
}

/*
 * RETURN
 *     1 if card present on slot
 */
int card_detect()
{
    if (gpio_read(&detect))
        return 0;
    else
        return 1;
}

