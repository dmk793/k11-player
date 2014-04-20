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

#ifndef USBDEV_H
#define USBDEV_H

#include <os.h>
#include "usbdev_hw.h"

void usbdev_task();

#define EP_MAX_PACKET_SIZE  64

struct usbdev_t {
#define USBDEV_EVENT_MASK_IRQ    (1 << 0) 
    BASE_TYPE event;

#define REALIZED_ENDPOINTS   2
    struct usbdev_ep_t {
        void (*out_cb)(struct usbdev_ep_t*, uint8 *, int);
//        int (*send)(struct usbdev_ep_t*, uint8 *, int, BASE_TYPE);

        uint8 epn;        /* endpoint number */
        int packsize;     /* maxmimum packet size of endpoint */
        struct {
            int p;        /* pointer for IN packet */
            int c;        /* counter for IN packet */
            uint8 *buf;   /* pointer for data */
            uint8 *abuf;  /* preallocated buffer */
        } in;

#define USBDEV_EVENT_MASK_EP_IN_READY   (1 << 0)
        BASE_TYPE event;
    } ep[REALIZED_ENDPOINTS];
};

extern struct usbdev_t usbdev;

int usbdev_send(struct usbdev_ep_t *ep, uint8 *payload, int len, BASE_TYPE timeout);
#endif

