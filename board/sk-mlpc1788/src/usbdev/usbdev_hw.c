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
#include <gpio.c>
#include <clk_cfg.h>
#include <os.h>
#include <debug.h>
#include "../irqp.h"
#include "usbdev.h"
#include "usbdev_proto.h"
#include "usbdev_hw.h"

static struct gpio_t connect_pin = {LPC_GPIO0, (1 << 14)};

static void pllfeed();

#define USBCLKSEL_USBDIV_CLKOFF    (0x00 << 0)
#define USBCLKSEL_USBDIV_DIV1      (0x01 << 0)
#define USBCLKSEL_USBDIV_DIV2      (0x02 << 0)
#define USBCLKSEL_USBDIV_DIV3      (0x03 << 0)

#define USBCLKSEL_USBSEL_SYSCLK    (0x00 << 8)
#define USBCLKSEL_USBSEL_PLL0      (0x01 << 8)
#define USBCLKSEL_USBSEL_PLL1      (0x02 << 8)

#define SIE_DIR_READ     0
#define SIE_DIR_WRITE    1
static void usbdev_hw_sie_cmd(uint8 cmd, uint8 dir, uint8 *data, uint8 len);
static void usbdev_hw_reset();
static int usbdev_hw_out_read(uint8 logep, uint8 *data, uint32 maxlen);

/*
 *
 */
void usbdev_hw_init()
{
    uint8 reg;

    os_disable_irq();
    {
        /*
         * configure pins for SK-MLPC1788 board
         *
         * USB_PWRD2/USB_VBUS     P1[30]    PIN42    UB_5V
         * USB_DP2                P0[31]    PIN51    UB_P
         * USB_DN2                          PIN52    UB_N
         * USB_CONNECT2           P0[14]    PIN69    UB_CON
         * USB_UP_LED2            P0[13]    PIN45    unused
         */
        LPC_IOCON_FUNC_SET(P1_30, LPC_IOCON_P1_30_USB_VBUS);
        LPC_IOCON_MODE_NO_PULLUP_NO_PULLDOWN(P1_30);

        LPC_IOCON_FUNC_SET(P0_31, LPC_IOCON_P0_31_USB_DP2);
//        LPC_IOCON_FUNC_SET(P0_14, LPC_IOCON_P0_14_USB_CONNECT2);
        gpio_setdir(&connect_pin, GPIO_DIR_OUTPUT);
        gpio_drive(&connect_pin, 0);

        /* power-up USB block */
        LPC_SC->PCONP |= PCONP_PCUSB;

        {
            /* disable usb clocks */
            LPC_SC->USBCLKSEL = 0;

            /* turn-off PLL */
            LPC_SC->PLL1CON = 0;
            pllfeed();

            /*
             * setup divider and multiplier
             *
             * 10    MHz  < pll_in_clk  < 25  MHz
             * 156   MHz  < Fcco        < 320 MHz
             * 9.75  MHz  < pll_out_clk < 160 MHz
             *
             *
             * M   1..32
             * P   1, 2, 4, 8
             *
             *
             * Fcco = pll_in_clk * M * 2 * P
             * pll_out_clk = Fcco / (2 * P)
             * 
             * M = pll_out_clk / pll_in_clk
             * 
             *
             */
            LPC_SC->PLL1CFG = (CLK_PLL1_M - 1) | ((CLK_PLL1_P - 1) << 5);
            pllfeed();
            /* enable PLL */
            LPC_SC->PLL1CON = PLLCON_PLLE;
            pllfeed();
            /* wait for enable */
            while(!(LPC_SC->PLL1STAT & PLLSTAT_PLLE));

            /* setup USB clock divider */
            LPC_SC->USBCLKSEL = USBCLKSEL_USBSEL_PLL1 | CLK_USBDIV /* XXX */;

            /* wait for the PLL to lock */
            while(!(LPC_SC->PLL1STAT & PLLSTAT_PLOCK));
        }

        {
            uint32 mask;

#define USB_CLKCTRL_DEV_CLK_EN       (1 << 1)
#define USB_CLKCTRL_PORTSEL_CLK_EN   (1 << 3)
#define USB_CLKCTRL_AHB_CLK_EN       (1 << 4)
            mask = 
                USB_CLKCTRL_PORTSEL_CLK_EN |
                USB_CLKCTRL_DEV_CLK_EN |
                USB_CLKCTRL_AHB_CLK_EN;
            LPC_USB->USBClkCtrl = mask;
#define USB_STCTRL_PORT_FUNC_U2_DEVICE    (3 << 0)
            while ((LPC_USB->USBClkSt & mask) != mask);
            LPC_USB->StCtrl = USB_STCTRL_PORT_FUNC_U2_DEVICE;
            LPC_USB->USBClkCtrl &= ~USB_CLKCTRL_PORTSEL_CLK_EN;
        }

#define USB_DEVINT_FRAME      (1 << 0)
#define USB_DEVINT_EP_FAST    (1 << 1)
#define USB_DEVINT_EP_SLOW    (1 << 2)
#define USB_DEVINT_DEV_STAT   (1 << 3)
#define USB_DEVINT_CCEMPTY    (1 << 4)
#define USB_DEVINT_CDFULL     (1 << 5)
#define USB_DEVINT_RXENDPKT   (1 << 6)
#define USB_DEVINT_TXENDPKT   (1 << 7)
#define USB_DEVINT_EP_RLZED   (1 << 8)
#define USB_DEVINT_ERR_INT    (1 << 9)

//        dprint("sn", "ep0 realized");

        usbdev_hw_reset();

        LPC_USB->DevIntEn =
            USB_DEVINT_DEV_STAT | USB_DEVINT_EP_SLOW;
//            USB_DEVINT_DEV_STAT | USB_DEVINT_ERR_INT | USB_DEVINT_EP_SLOW;

        /* enable device */
        reg = SIE_DEV_EN;
        usbdev_hw_sie_cmd(SIE_CMD_DEV_SET_ADDRESS, SIE_DIR_WRITE, &reg, 1);
        /* connect, NOTE not necessary, connect with GPIO is used instead */
        reg = SIE_DEV_STAT_CON;
        usbdev_hw_sie_cmd(SIE_CMD_DEV_STATUS, SIE_DIR_WRITE, &reg, 1);

//        /* always PLL clock, both successfull and NAKed transactions generate interrupts */
//        reg = 0xef;
        /* always PLL clock */
        reg = 0x01;
        usbdev_hw_sie_cmd(SIE_CMD_DEV_SET_MODE, SIE_DIR_WRITE, &reg, 1);

        /* pull-up line with resistor */
        gpio_drive(&connect_pin, 1);

        NVIC_SetPriority(USB_IRQn, USB_IRQP);
        NVIC_EnableIRQ(USB_IRQn);
    }
    os_enable_irq();
}

/*
 * PLL feed sequence
 */
static void pllfeed()
{
    LPC_SC->PLL1FEED = 0xAA;
    LPC_SC->PLL1FEED = 0x55;
}

/*
 *
 */
void USB_Handler(void)
{
    os_event_raise(&usbdev.event, USBDEV_EVENT_MASK_IRQ);
    NVIC_DisableIRQ(USB_IRQn);
}

/*
 * bottom half of USB interrupt
 */
void usbdev_hw_bh()
{
    uint32 devintst;
    int i;

    devintst = LPC_USB->DevIntSt;
//    dprint("s4xn", "usb_handler ", devintst);
    if (devintst & USB_DEVINT_DEV_STAT)
    {
        uint8 devstat;

        LPC_USB->DevIntClr = USB_DEVINT_DEV_STAT;

        usbdev_hw_sie_cmd(SIE_CMD_DEV_STATUS, SIE_DIR_READ, &devstat, 1);
//        dprint("s1xn", "devstat = ", devstat);

        if (devstat & SIE_DEV_STAT_RST)
            usbdev_hw_reset();
    }

//    if (devintst & USB_DEVINT_ERR_INT)
//    {
//        uint8 err;
//
//        LPC_USB->DevIntClr = USB_DEVINT_ERR_INT;
//
//        usbdev_hw_sie_cmd(SIE_CMD_DEV_READ_ERROR_STATUS, SIE_DIR_READ, &err, 1);
//        dprint("s1xn", "err = ", err);
//    }

    if (devintst & USB_DEVINT_EP_SLOW)
    {
        uint32 epintst;

        LPC_USB->DevIntClr = USB_DEVINT_EP_SLOW;

        epintst = LPC_USB->EpIntSt;

        /* process endpoints transactions */
        for (i = 0; i < REALIZED_ENDPOINTS; i++)
        {
            struct usbdev_ep_t *ep;
            uint32 epn;
            uint8 buf[EP_MAX_PACKET_SIZE];
            int len;

            ep = &usbdev.ep[i];

            /* OUT */
            epn = ep->epn & 0x0f;
            if (epintst & EP2MASK(epn))
            {
                LPC_USB->EpIntClr = EP2MASK(epn);
                while (!(LPC_USB->DevIntSt & USB_DEVINT_CDFULL));

                len = usbdev_hw_out_read(epn, buf, ep->packsize);
                if (len > 0)
                {
                    if (epn == 0)
                        usbdev_proto_control_ep_OUT(buf, len);
                    else
                        usbdev_proto_ep_OUT(ep, buf, len);
                }
                continue;
            }

            /* IN */
            epn = ep->epn | 0x80;
            if (epintst & EP2MASK(epn))
            {
                LPC_USB->EpIntClr = EP2MASK(epn);
                while (!(LPC_USB->DevIntSt & USB_DEVINT_CDFULL));

                usbdev_proto_ep_IN(ep);
            }


//            uint32 epintst;
//            uint32 epn;
//            uint8 buf[EP_MAX_PACKET_SIZE];
//            int len;
//
//            epintst = LPC_USB->EpIntSt;
//
//            /* OUT */
//            epn = i;
//            if (epintst & EP2MASK(epn))
//            {
//                LPC_USB->EpIntClr = EP2MASK(epn);
//                while (!(LPC_USB->DevIntSt & USB_DEVINT_CDFULL));
//
//                len = usbdev_hw_out_read(epn, buf, EP_MAX_PACKET_SIZE);
//                if (len > 0)
//                {
//                    if (epn == 0)
//                        usbdev_proto_control_ep_OUT(buf, len);
//                    else
//                        usbdev_proto_ep_OUT(epn, buf, len);
//                }
//                continue;
//            }
//
//            /* IN */
//            epn = i | 0x80;
//            if (epintst & EP2MASK(epn))
//            {
//                LPC_USB->EpIntClr = EP2MASK(epn);
//                while (!(LPC_USB->DevIntSt & USB_DEVINT_CDFULL));
//
//                usbdev_proto_ep_IN(epn);
//            }
        }
    }

    NVIC_EnableIRQ(USB_IRQn);
}

/* called when reset occured */
static void usbdev_hw_reset()
{
    uint8 reg;
    int i, n;

    /* clear all interrupts */
    LPC_USB->DevIntClr = 0x3ff;
    LPC_USB->EpIntClr  = 0xffffffff;
    LPC_USB->EpIntEn   = 0x00000000;

    for (i = 0; i < REALIZED_ENDPOINTS; i++)
    {
        /* realize both IN and OUT endpoints */
        for (n = 0; n < 2; n++)
        {
            uint8 epn;

            epn = usbdev.ep[i].epn;
            if (n == 0)
                epn &= 0x0f;
            else
                epn |= 0x80;

            LPC_USB->ReEp    |= EP2MASK(epn);
            LPC_USB->EpInd    = EP2IDX(epn);
            LPC_USB->MaxPSize = usbdev.ep[i].packsize;

            while (!(LPC_USB->DevIntSt & USB_DEVINT_EP_RLZED));

            LPC_USB->EpIntEn |= EP2MASK(epn);
        }
    }

    reg = SIE_DEV_EN;
    usbdev_hw_sie_cmd(SIE_CMD_DEV_SET_ADDRESS, SIE_DIR_WRITE, &reg, 1);
}

/*
 *
 */
static void usbdev_hw_sie_cmd(uint8 cmd, uint8 dir, uint8 *data, uint8 len)
{
    LPC_USB->DevIntClr = USB_DEVINT_CCEMPTY | USB_DEVINT_CDFULL;
    LPC_USB->CmdCode   = (cmd << 16) | SIE_CMD_PHASE_COMMAND;
    while (!(LPC_USB->DevIntSt & USB_DEVINT_CCEMPTY));
    LPC_USB->DevIntClr = USB_DEVINT_CCEMPTY;

    if (data == NULL)
        return;

    if (dir == SIE_DIR_READ)
    {
        while (len--)
        {
            LPC_USB->CmdCode = (cmd << 16) | SIE_CMD_PHASE_READ;
            while (!(LPC_USB->DevIntSt & USB_DEVINT_CDFULL));
            LPC_USB->DevIntClr = USB_DEVINT_CDFULL;
            *data++ = LPC_USB->CmdData & 0xff;
        }
    } else {
        while (len--)
        {
            LPC_USB->CmdCode = ((*data++) << 16) | SIE_CMD_PHASE_WRITE;
            while (!(LPC_USB->DevIntSt & USB_DEVINT_CCEMPTY));
            LPC_USB->DevIntClr = USB_DEVINT_CCEMPTY;
        }
    }
}

/*
 *
 */
static int usbdev_hw_out_read(uint8 epn, uint8 *data, uint32 maxlen)
{
    int c;
    int i;
    uint32 rxplen;
    uint32 rxdata;
    uint8 reg;

    LPC_USB->Ctrl = USB_CTRL_RD_EN | ((epn & 0x0f) << 2);

    do {
        rxplen = LPC_USB->RxPLen;
    } while (!(rxplen & USB_RXPLEN_PKT_RDY));

    if (!(rxplen & USB_RXPLEN_DV))
    {
        return -1;
    }

    c = 0;
    rxplen &= 0x3ff; /* PKT_LENGTH */
    while (LPC_USB->Ctrl & USB_CTRL_RD_EN)
    {
        rxdata = LPC_USB->RxData;
        for (i = 0; maxlen && i < 4; i++, maxlen--)
        {
            if (rxplen-- == 0)
                break;
            *data++ = rxdata & 0xff;
            c++;
            rxdata >>= 8;
        }
    }

    usbdev_hw_sie_cmd(SIE_CMD_EP_SELECT + EP2IDX(epn), 0, NULL, 0);
    usbdev_hw_sie_cmd(SIE_CMD_EP_CLEAR_BUFFER, SIE_DIR_READ, &reg, 1);
    /* NOTE only for control endpoint */
    if (reg & SIE_EP_CLEAR_BUF_PO)
    {
        usbdev_hw_sie_cmd(SIE_CMD_EP_SELECT_CLEAR + EP2IDX(epn), SIE_DIR_READ, &reg, 1);
        return -1;
    }
    return c;
}

/*
 *
 */
void usbdev_hw_in_write(uint8 epn, uint8 *data, uint32 len)
{
    LPC_USB->Ctrl   = USB_CTRL_WR_EN | ((epn & 0x0f) << 2);
    LPC_USB->TxPLen = len;

    while (LPC_USB->Ctrl & USB_CTRL_WR_EN)
    {
        uint32 word;

        word  = (*data++) << 0;
        word |= (*data++) << 8;
        word |= (*data++) << 16;
        word |= (*data++) << 24;

        LPC_USB->TxData = word;
    }

    usbdev_hw_sie_cmd(SIE_CMD_EP_SELECT + EP2IDX(epn), 0, NULL, 0);
    usbdev_hw_sie_cmd(SIE_CMD_EP_VALIDATE_BUFFER, 0, NULL, 0);
}

/*
 *
 */
void usbdev_hw_stall_ep(uint8 epn)
{
    uint8 reg;

    reg = SIE_EP_STATUS_ST;
    usbdev_hw_sie_cmd(SIE_CMD_EP_SELECT_CLEAR + EP2IDX(epn), SIE_DIR_WRITE, &reg, 1);
}

/*
 *
 */
void usbdev_hw_setaddr(uint8 addr)
{
    uint8 reg;

    reg = SIE_DEV_EN | addr;
    usbdev_hw_sie_cmd(SIE_CMD_DEV_SET_ADDRESS, SIE_DIR_WRITE, &reg, 1);
}

/*
 *
 */
void usbdev_hw_setconfig()
{
    uint8 reg;

    reg = SIE_DEV_CONFIG_CONF_DEVICE;
    usbdev_hw_sie_cmd(SIE_CMD_DEV_CONFIG, SIE_DIR_WRITE, &reg, 1);
}

