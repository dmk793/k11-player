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
#include <stimer.h>
#include <debug.h>
#include <os.h>
#include "irqp.h"
#include "dma.h"

#define DMA_CHAN_MASK(chan) (1 << (chan))
#define DMA_ALLCHAN_MASK    0xff
static BASE_TYPE dma_free;

struct dma_lli_t {
    uint32 srcaddr;
    uint32 dstaddr;
    uint32 next;
    uint32 control;
} __attribute__((packed));

struct dma_chan_t {
    volatile LPC_GPDMACH_TypeDef *p;
#define DMA_LLI_NUM    300
    struct dma_lli_t lli[DMA_LLI_NUM];
};

#define DMA_CHAN_NUM   8
struct dma_chan_t dma_chan[DMA_CHAN_NUM];

#define DMA_CCONTROL_TRANSFERSIZE(val) (val << 0)  /* 11:0 */
#define DMA_CCONTROL_TRANSFERSIZE_MASK (0x0fff << 0)  /* 11:0 */
#define DMA_CCONTROL_SBSIZE(val)       (val << 12) /* 14:12, source birst size */
#define DMA_CCONTROL_SBSIZE_1          (0 << 12)   /* 14:12 */
#define DMA_CCONTROL_SBSIZE_4          (1 << 12)   /* 14:12 */
#define DMA_CCONTROL_SBSIZE_8          (2 << 12)   /* 14:12 */
#define DMA_CCONTROL_SBSIZE_16         (3 << 12)   /* 14:12 */
#define DMA_CCONTROL_SBSIZE_32         (4 << 12)   /* 14:12 */
#define DMA_CCONTROL_SBSIZE_64         (5 << 12)   /* 14:12 */
#define DMA_CCONTROL_SBSIZE_128        (6 << 12)   /* 14:12 */
#define DMA_CCONTROL_SBSIZE_256        (7 << 12)   /* 14:12 */
#define DMA_CCONTROL_DBSIZE(val)       (val << 15) /* 14:12, destination birst size */
#define DMA_CCONTROL_DBSIZE_1          (0 << 15)   /* 17:15 */
#define DMA_CCONTROL_DBSIZE_4          (1 << 15)   /* 17:15 */
#define DMA_CCONTROL_DBSIZE_8          (2 << 15)   /* 17:15 */
#define DMA_CCONTROL_DBSIZE_16         (3 << 15)   /* 17:15 */
#define DMA_CCONTROL_DBSIZE_32         (4 << 15)   /* 17:15 */
#define DMA_CCONTROL_DBSIZE_64         (5 << 15)   /* 17:15 */
#define DMA_CCONTROL_DBSIZE_128        (6 << 15)   /* 17:15 */
#define DMA_CCONTROL_DBSIZE_256        (7 << 15)   /* 17:15 */
#define DMA_CCONTROL_SWIDTH(val)       (val << 18) /* 20:18, source transfer width */
#define DMA_CCONTROL_SWIDTH_BYTE       (0 << 18)   /* 20:18, 8-bit */
#define DMA_CCONTROL_SWIDTH_HWORD      (1 << 18)   /* 20:18, 16-bit */
#define DMA_CCONTROL_SWIDTH_WORD       (2 << 18)   /* 20:18, 32-bit */
#define DMA_CCONTROL_DWIDTH(val)       (val << 21) /* 23:21, destination transfer width */
#define DMA_CCONTROL_DWIDTH_BYTE       (0 << 21)   /* 23:21, 8-bit */
#define DMA_CCONTROL_DWIDTH_HWORD      (1 << 21)   /* 23:21, 16-bit */
#define DMA_CCONTROL_DWIDTH_WORD       (2 << 21)   /* 23:21, 32-bit */
#define DMA_CCONTROL_SI                (1 << 26)   /*    26, source increment */
#define DMA_CCONTROL_DI                (1 << 27)   /*    27, destination increment */
#define DMA_CCONTROL_I                 (1 << 31)   /*    31, terminal count interrupt enable */

#define DMA_CCONFIG_E                                        (1 << 0)    /*     0, channel enable */
#define DMA_CCONFIG_SRCPERIPHERAL(val)                       (val << 1)  /*  5: 1, source peripheral */
#define DMA_CCONFIG_DESTPERIPHERAL(val)                      (val << 6)  /* 10: 6, destinaiton peripheral */
#define DMA_CCONFIG_TRANSFERTYPE(val)                        (val << 11) /* 13:11, type of transfer and flow controller */
#define DMA_CCONFIG_TRANSFERTYPE_MEMORY2MEMORY               (0   << 11)
#define DMA_CCONFIG_TRANSFERTYPE_PERIPH2MEMORY_FLOW_DMA      (2   << 11)
#define DMA_CCONFIG_TRANSFERTYPE_PERIPH2MEMORY_FLOW_PERIPH   (6   << 11)
#define DMA_CCONFIG_IE                                       (1   << 14) /*    14, when cleared, masks out error interrupt  */
#define DMA_CCONFIG_ITC                                      (1   << 15) /*    15, when cleared, masks out terminal count interrupt */
#define DMA_CCONFIG_A                                        (1   << 17) /*    17, 1 - channel fifo has data */
#define DMA_CCONFIG_H                                        (1   << 18) /*    18, 1 - ignore further source DMA requests */

/*
 *
 */
void dma_init()
{
    int i;

    LPC_SC->PCONP |= PCONP_PCGPDMA;

    LPC_GPDMA->Config = (1 << 0); /* enable DMA controller */

    NVIC_EnableIRQ(DMA_IRQn);
    NVIC_SetPriority(DMA_IRQn, DMA_IRQP);
    dma_free = DMA_ALLCHAN_MASK;

    for (i = 0; i < DMA_CHAN_NUM; i++)
    {
        switch (i)
        {
            case 0x00: dma_chan[i].p = LPC_GPDMACH0; break;
            case 0x01: dma_chan[i].p = LPC_GPDMACH1; break;
            case 0x02: dma_chan[i].p = LPC_GPDMACH2; break;
            case 0x03: dma_chan[i].p = LPC_GPDMACH3; break;
            case 0x04: dma_chan[i].p = LPC_GPDMACH4; break;
            case 0x05: dma_chan[i].p = LPC_GPDMACH5; break;
            case 0x06: dma_chan[i].p = LPC_GPDMACH6; break;
            case 0x07: dma_chan[i].p = LPC_GPDMACH7; break;
        }
    }
}

/*
 *
 */
struct dma_chan_t *dma_mask2ch(int mask)
{
    struct dma_chan_t *dmach;

    /* select channel */
    switch (mask)
    {
        case 0x01: dmach = &dma_chan[0]; break;
        case 0x02: dmach = &dma_chan[1]; break;
        case 0x04: dmach = &dma_chan[2]; break;
        case 0x08: dmach = &dma_chan[3]; break;
        case 0x10: dmach = &dma_chan[4]; break;
        case 0x20: dmach = &dma_chan[5]; break;
        case 0x40: dmach = &dma_chan[6]; break;
        case 0x80: dmach = &dma_chan[7]; break;
        default:
            /* NOTREACHED */
            dprint("s1xn", ERR_PREFIX"DMA unknown chan, mask ", mask);
            return NULL;
    }

    return dmach;
}

/*
 *
 */
void DMA_Handler()
{
    int mask;

//    dprint("sn", "DMA Handler");
    for (mask = 0x80; mask != 0x00; mask >>= 1)
    {
        if (LPC_GPDMA->IntStat & mask)
        {
            if (LPC_GPDMA->IntTCStat & mask)
            {
                LPC_GPDMA->IntTCClear = mask;
                os_event_raise(&dma_free, mask);
                continue;
            }
            if (LPC_GPDMA->IntErrStat & mask)
            {
                LPC_GPDMA->IntErrClr = mask;
                /* NOTE
                 * Event should not raised in such situation.
                 * Timeout in dma_wait_chan() will signal process
                 * for error occurence.
                 *
                 * Or raise other event (error bit).
                 */
                dprint("sn", "(E) DMA error");
//                os_event_raise(&dma_free, mask);
                continue;
            }
        }
    }
}

/*
 * wait when DMA channel becomes free
 *
 * ARGS
 *     return 1 on success, 0 on timeout
 */
int dma_wait_chan(int chmask, uint32 to)
{
    struct dma_chan_t *dmach;

    dmach = dma_mask2ch(chmask);

    /* wait when DMA become free */
    if (os_event_wait(&dma_free, chmask, OS_FLAG_NONE, OS_MS2TICK(to)) == OS_ERR_TIMEOUT)
    {
        dprint("s1xn", "(E) dma wait timeout, ch ", chmask);

        /* disable and halt DMA channel */
        if (dmach->p->CConfig & DMA_CCONFIG_A)
        {
            dmach->p->CConfig |= DMA_CCONFIG_H;
            while (dmach->p->CConfig & DMA_CCONFIG_A)
                os_wait(1);
            dmach->p->CConfig = 0;
        }

        os_event_raise(&dma_free, chmask);
        return 0;
    }

    asm volatile ("dmb\r\n"); /* NOTE */

    return 1;
}

/*
 * copy window-like memory region with DMA
 *
 * ARGS
 *
 * RETURN
 *     mask of DMA channel that was used
 */
int dma_copy_window(void *dst, uint32 dstwidth, void *src, uint32 srclen, uint32 srcwidth, uint32 lines)
{
    int mask;
    struct dma_chan_t *dmach;
    uint32 srcaddr, dstaddr;
    uint32 control, lli;
    int i;

    if (!lines)
        return 0;

    mask = DMA_CHMASK_WINDOW;

    /* clear event */
    os_event_wait(&dma_free, mask, OS_FLAG_CLEAR, OS_WAIT_FOREVER);

    /* NOTE */
    LPC_GPDMA->IntTCClear = mask;
    LPC_GPDMA->IntErrClr = mask;

    /* sanity check */
    if (LPC_GPDMA->EnbldChns & mask)
    {
        /* NOTREACHED */
        dprint("s1xn", ERR_PREFIX "DMA chan still busy, mask ", mask);
        dprint("s1xn", ERR_PREFIX "free ", dma_free);
        return 0;
    }

    dmach = dma_mask2ch(mask);
    if (!dmach)
        return 0;

    /* fill linked list with addreses of lines */
    dstaddr = (uint32)dst;
    srcaddr = (uint32)src;

    dmach->p->CSrcAddr  = srcaddr;
    dmach->p->CDestAddr = dstaddr;
    control = 
        DMA_CCONTROL_TRANSFERSIZE(srclen) |
        DMA_CCONTROL_SBSIZE_4 |
        DMA_CCONTROL_DBSIZE_4 |
        DMA_CCONTROL_SWIDTH_BYTE |
        DMA_CCONTROL_DWIDTH_BYTE |
        DMA_CCONTROL_SI |
        DMA_CCONTROL_DI;
    lines--;
    dstaddr += dstwidth;
    srcaddr += srcwidth;
    if (lines == 0) {
        lli = 0;
        control |= DMA_CCONTROL_I;
    } else {
        lli = (uint32)dmach->lli;
        for (i = 0; i < (lines - 1); i++)
        {
            /* NOTE check that lines number does not exceed maximum linked list size */
            if (i >= (DMA_LLI_NUM - 1))
                break;

            dmach->lli[i].srcaddr = srcaddr;
            dmach->lli[i].dstaddr = dstaddr;
            dmach->lli[i].next    = (uint32)&dmach->lli[i + 1];
            dmach->lli[i].control = control;

            dstaddr += dstwidth;
            srcaddr += srcwidth;
        }
        dmach->lli[i].srcaddr = srcaddr;
        dmach->lli[i].dstaddr = dstaddr;
        dmach->lli[i].next    = 0;
        dmach->lli[i].control = control | DMA_CCONTROL_I;
    }

    asm volatile ("dmb\r\n"); /* NOTE */

    dmach->p->CLLI     = lli;
    dmach->p->CControl = control;
    dmach->p->CConfig  =
        DMA_CCONFIG_E | DMA_CCONFIG_TRANSFERTYPE_MEMORY2MEMORY |
        DMA_CCONFIG_IE | DMA_CCONFIG_ITC;

    asm volatile ("dmb\r\n"); /* NOTE */

    dma_wait_chan(mask, 0); /* NOTE wait forever */

    return mask;
}

/*
 *
 */
int dma_sd_read(void *dst)
{
    int mask;
    struct dma_chan_t *dmach;

    mask = DMA_CHMASK_SD;

    /* clear event */
    os_event_wait(&dma_free, mask, OS_FLAG_CLEAR, OS_WAIT_FOREVER);

    /* NOTE */
    LPC_GPDMA->IntTCClear = mask;
    LPC_GPDMA->IntErrClr = mask;

    /* sanity check */
    if (LPC_GPDMA->EnbldChns & mask)
    {
        /* NOTREACHED */
        dprint("s1xn", ERR_PREFIX "DMA chan still busy, mask ", mask);
        dprint("s1xn", ERR_PREFIX "free ", dma_free);
        return 0;
    }

    dmach = dma_mask2ch(mask);
    if (!dmach)
    {
        dprint("sn", ERR_PREFIX "dmach error");
        return 0;
    }


    dmach->p->CSrcAddr  = (uint32)&LPC_MCI->FIFO;
    dmach->p->CDestAddr = (uint32)dst;
    dmach->p->CLLI     = 0;

    /* NOTE TRANSFERSIZE not used if peripheral is flow controller */
    dmach->p->CControl = 
        DMA_CCONTROL_TRANSFERSIZE(512) |
        DMA_CCONTROL_SBSIZE_8 |
        DMA_CCONTROL_DBSIZE_8 |
        DMA_CCONTROL_SWIDTH_WORD |
        DMA_CCONTROL_DWIDTH_WORD |
        DMA_CCONTROL_DI |
        DMA_CCONTROL_I;
    dmach->p->CConfig  =
        DMA_CCONFIG_SRCPERIPHERAL(DMA_REQUEST_SD) |
        DMA_CCONFIG_E |
        DMA_CCONFIG_TRANSFERTYPE_PERIPH2MEMORY_FLOW_PERIPH |
        DMA_CCONFIG_IE | 
        DMA_CCONFIG_ITC;

    asm volatile ("dmb\r\n"); /* NOTE */

//#define SD_CARD_DMA_TIMEOUT   200
//    if (!dma_wait_chan(DMA_CHMASK_SD, SD_CARD_DMA_TIMEOUT))
//        return 0;

    return mask;
}

/*
 * cancel SD card dma transfer
 */
void dma_sd_cancel()
{
    int mask;
    struct dma_chan_t *dmach;

    mask = DMA_CHMASK_SD;

    dmach = dma_mask2ch(mask);
    if (!dmach)
    {
        dprint("sn", ERR_PREFIX "dmach error");
        return;
    }
    dmach->p->CConfig &= ~DMA_CCONFIG_E;
}

