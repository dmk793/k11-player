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

#include <os.h>
#include <string.h>
#include <debug.h>
#include "../upload.h"
#include "usbdev.h"
#include "usbdev_proto.h"

struct usbdev_t usbdev;

#define USBDEV_MAX_BUF_SIZE    512

/*
 *
 */
void usbdev_task()
{
    usbdev.ep[0].epn      = 0x00;
    usbdev.ep[0].packsize = 8;

    usbdev.ep[1].epn      = 0x02;
    usbdev.ep[1].packsize = 64;
    usbdev.ep[1].event   |= USBDEV_EVENT_MASK_EP_IN_READY;
    usbdev.ep[1].out_cb   = upload_usb_receive;
    usbdev.ep[1].in.abuf  = os_malloc(USBDEV_MAX_BUF_SIZE);

    usbdev_hw_init();

    while (1)
    {
        if (os_event_wait(&usbdev.event, USBDEV_EVENT_MASK_IRQ,
                    OS_FLAG_CLEAR, OS_WAIT_FOREVER) == OS_ERR_NONE)
            usbdev_hw_bh();
    }
}

/*
 * XXX use only from one task or add mutex
 *
 * RETURN
 *     <0 on error, count of transfered bytes otherwise
 */
int usbdev_send(struct usbdev_ep_t *ep, uint8 *payload, int len, BASE_TYPE timeout)
{
    int ret;

    ret = 0;

    if (ep->in.abuf == NULL)
        return -1;

    if (len > USBDEV_MAX_BUF_SIZE)
        return -2;

    /* TODO check busy */
//    if (os_event_wait(&ep->event,
//                USBDEV_EVENT_MASK_EP_IN_READY, OS_FLAG_CLEAR, timeout) != OS_ERR_NONE)
//    {
//        /* TODO free endpoint (stall/unstall) ? */
//        return -3;
//    }

    memcpy(ep->in.abuf, payload, len);

    ep->in.p   = 0;
    ep->in.c   = len;
    ep->in.buf = ep->in.abuf;
    usbdev_proto_send_IN(ep, 0);

    return ret;
}

