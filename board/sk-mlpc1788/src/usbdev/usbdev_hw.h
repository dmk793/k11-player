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
#ifndef USBDEVHW_H
#define USBDEVHW_H

void usbdev_hw_init();
void usbdev_hw_bh();

//#define EP0_PACKET_SIZE     8
//#define EP1_PACKET_SIZE     64
//#define EP2_PACKET_SIZE     64

/*****************************************
 * SIE engine commands and bits
 *****************************************/
/* device commands */
#define SIE_CMD_DEV_SET_ADDRESS            0xD0
#define SIE_CMD_DEV_CONFIG                 0xD8
#define SIE_CMD_DEV_SET_MODE               0xF3
#define SIE_CMD_DEV_READ_CUR_FRAME_NR      0xF5
#define SIE_CMD_DEV_READ_TEST_REG          0xFD
#define SIE_CMD_DEV_STATUS                 0xFE /* read/write */
#define SIE_CMD_DEV_GET_ERROR_CODE         0xFF
#define SIE_CMD_DEV_READ_ERROR_STATUS      0xFB
/* endpoint commands */
#define SIE_CMD_EP_SELECT                  0x00
#define SIE_CMD_EP_SELECT_CLEAR            0x40
#define SIE_CMD_EP_SET_STATUS              0x40
#define SIE_CMD_EP_CLEAR_BUFFER            0xF2
#define SIE_CMD_EP_VALIDATE_BUFFER         0xFA

#define SIE_CMD_PHASE_READ      (0x02 << 8)
#define SIE_CMD_PHASE_WRITE     (0x01 << 8)
#define SIE_CMD_PHASE_COMMAND   (0x05 << 8)

/* SIE_CMD_DEV_SET_ADDRESS */
#define SIE_DEV_EN    (1 << 7)

/* SIE_CMD_DEV_CONFIG */
#define SIE_DEV_CONFIG_CONF_DEVICE    (1 << 0)

/* SIE_CMD_DEV_STATUS */
#define SIE_DEV_STAT_CON    (1 << 0) /* current connection status of device */        
#define SIE_DEV_STAT_CON_CH (1 << 1) /* connect change, VBUS dissappeared */
#define SIE_DEV_STAT_SUS    (1 << 2) /* suspend */
#define SIE_DEV_STAT_SUS_CH (1 << 3) /* suspend change */
#define SIE_DEV_STAT_RST    (1 << 4) /* bus reset */

#define PHYS_EP_OUT(logic)   (1 << (logic << 1))
#define PHYS_EP_IN(logic)    (1 << ((logic << 1) + 1))

/* SIE_CMD_EP_CLEAR_BUFFER */
#define SIE_EP_CLEAR_BUF_PO    (1 << 0)
 
#define EP2MASK(epn) (1 << (          \
            ((epn & 0x0f) << 1) + (   \
                (epn & 0x80) >> 7))   \
            )

/* SIE_CMD_EP_SET_STATUS */
#define SIE_EP_STATUS_ST (1 << 0) /* stall endpoint */

/** convert from endpoint address to endpoint index */
#define EP2IDX(epn)     ((((epn)&0xF)<<1)|(((epn)&0x80)>>7))
/** convert from endpoint index to endpoint address */
#define IDX2EP(idx)     ((((idx)<<7)&0x80)|(((idx)>>1)&0xF))

/*****************************************
 *
 *****************************************/
#define USB_CTRL_RD_EN       (1 << 0)    /* enables reading data from OUT endpoint */
#define USB_CTRL_WR_EN       (1 << 1)    /* enables writing data to IN endpoint */
/*****************************************
 *
 *****************************************/
#define USB_RXPLEN_DV        (1 << 10) /* data valid */
#define USB_RXPLEN_PKT_RDY   (1 << 11)


void usbdev_hw_stall_ep(uint8 epn);
void usbdev_hw_in_write(uint8 epn, uint8 *data, uint32 len);
void usbdev_hw_setaddr(uint8 addr);
void usbdev_hw_setconfig();

#endif

