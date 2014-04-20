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

#include "debug.h"
#include "usbdev.h"
#include "usbdev_hw.h"
#include "usbdev_proto.h"

#define USBDEBUG

#ifdef USBDEBUG
    #define DPRINT(fmt, ...) dprint(fmt, __VA_ARGS__)
#else
    #define DPRINT(fmt, ...)
#endif


#define VID  0xA598
#define PID  0x0002

/* XXX check endpoint packet size */
const struct USB_Device_Descriptor device_desc = {
    18,                            /* Size of this descriptor in bytes */
    DESC_TYPE_DEVICE,              /* Device descriptor type */
    0x0200,                        /* USB Spec Release Number in BCD format */
    0xff,                          /* Class Code */
    0x00,                          /* Subclass code */
    0xff,                          /* Protocol code */
    8,                             /* Max packet size for EP0 */
    VID,                           /* Vendor ID */
    PID,                           /* Product ID */
    0x0100,                        /* Device release number in BCD format */
    0,                             /* Manufacturer string index */
    0,                             /* Product string index */
    0,                             /* Device serial number string index */
    1                              /* Number of possible configurations */
};

const uint8 configuration[] = {
    /* Configuration */
    0x09,                       /* Descriptor length (9 bytes)              */
    0x02,                       /* Descriptor type (Configuration)          */
    32, 0x00,                   /* Total data length (32 bytes)             */
    0x01,                       /* Interface supported (1)                  */
    0x01,                       /* Configuration value (1)                  */
    0x00,                       /* Index of string descriptor (None)        */
    0x80,                       /* Configuration (no self powered )         */
    250,                        /* XXX Maximum power consumption (500mA)    */
    /* Interface */
    0x09,                       /* Descriptor length (9 bytes)        */
    0x04,                       /* Descriptor type (Interface)        */
    0x00,                       /* Number of interface (0)            */
    0x00,                       /* Alternate setting (0)              */
    0x02,                       /* Number of interface endpoint (2)   */
    0xff,                       /* Class code                         */
    0x00,                       /* Subclass code ()                   */
    0xff,                       /* Protocol code ()                   */
    0x00,                       /* Index of string()                  */
    /* Endpoint */
    0x07,                       /* Descriptor length (7 bytes)              */
    0x05,                       /* Descriptor type (Endpoint)               */
    0x82,                       /* Encoded address (Respond to IN) EP1      */
    0x02,                       /* Endpoint attribute (Bulk transfer)       */
//    0x03,                       /* Endpoint attribute (Interrupt transfer)  */
    (64 & 0xff),                /* Maximum packet size                      */
    (64 >> 8),
    0x01,                       /* Polling interval                         */
    /* Endpoint */
    0x07,                       /* Descriptor length (7 bytes)              */
    0x05,                       /* Descriptor type (Endpoint)               */
    0x02,                       /* Encoded address (Respond to OUT) EP1     */
    0x02,                       /* Endpoint attribute (Bulk transfer)       */
//    0x03,                       /* Endpoint attribute (Interrupt transfer)  */
    (64 & 0xff),                /* Maximum packet size                      */
    (64 >> 8),
    0x01,                       /* Polling interval                         */
};

static int setaddr;

/*
 * process OUT transaction on EP0
 */
void usbdev_proto_control_ep_OUT(uint8 *buf, int len)
{
    struct dev_req_t *dr;
    struct usbdev_ep_t *ep;

    ep = &usbdev.ep[0];
    dr = (struct dev_req_t *)buf;

//    dprint("s1dsn", "devreq (", len, ")");
//    dprint("ts1xn", "bmRequestType ", dr->bmRequestType);
//    dprint("ts1xn", "bRequest      ", dr->bRequest);
//    dprint("ts2xn", "wValue        ", dr->wValue);
//    dprint("ts2xn", "wIndex        ", dr->wIndex);
//    dprint("ts2xn", "wLength       ", dr->wLength);

    if ((dr->bmRequestType == 0x80) && dr->bRequest == GET_DESCRIPTOR)
    {
        uint8 desc  = (dr->wValue >> 8);
//        uint8 index = dr->wValue & 0xff;

        if (desc == DESC_TYPE_DEVICE)
        {
            ep->in.p   = 0;
            ep->in.c   = sizeof(struct USB_Device_Descriptor);
            ep->in.buf = (uint8*)&device_desc;
            usbdev_proto_send_IN(ep, 0);
            DPRINT("s", "gD");
        } else if (desc == DESC_TYPE_DEVICE_QUALIFIER) {
            /* we operate only on full speed, send stall */
            usbdev_proto_send_IN(ep, SET_STALL);
            DPRINT("s", "gQ");
        } else if (desc == DESC_TYPE_CONFIGURATION) {
            ep->in.p   = 0;
            if (dr->wLength == sizeof(configuration))
                ep->in.c = sizeof(configuration);
            else
                ep->in.c = 9; /* XXX */
            ep->in.buf = (uint8*)configuration;
            usbdev_proto_send_IN(ep, 0);
            DPRINT("s", "gC");
        } else {
            DPRINT("s", "gU");
        }
    } else if ((dr->bmRequestType == 0x00) && dr->bRequest == SET_ADDRESS) {
        DPRINT("s", "sA");
        ep->in.p = 0;
        ep->in.c = 0;
        ep->in.buf = NULL;
        usbdev_proto_send_IN(ep, 0);
        setaddr = dr->wValue & 0xff;
        usbdev_hw_setaddr(setaddr);
    } else if ((dr->bmRequestType == 0x00) && dr->bRequest == SET_CONFIGURATION) {
        DPRINT("s", "sC");

        /* TODO valid processing */
        usbdev_hw_setconfig();

        ep->in.p   = 0;
        ep->in.c   = 0;
        ep->in.buf = NULL;
        usbdev_proto_send_IN(ep, 0);
    } else {
        /* unknown device request */
        DPRINT("s", "uDR");
    }    
}

/*
 * send control endpoint data to host
 */
void usbdev_proto_send_IN(struct usbdev_ep_t *ep, uint8 stall)
{
    int c;
    int p;
    uint8 *buf;

    if (stall)
    {
        usbdev_hw_stall_ep(ep->epn | 0x80);
        return;
    }

    if (ep->in.c < 0)
        return; /* UNREACHED */

    c   = ep->in.c;
    p   = ep->in.p;
    buf = &ep->in.buf[p];

    if (c > ep->packsize)
        c = ep->packsize;

    usbdev_hw_in_write(ep->epn | 0x80, buf, c);

    ep->in.c -= c;
    ep->in.p += c;
}

/*
 * process OUT transactions on EP1, EP2, ...
 */
void usbdev_proto_ep_OUT(struct usbdev_ep_t *ep, uint8 *buf, int len)
{
//    dprint("s1xsn", "t ", ep->epn, " OUT");
//    dprint("slb", "out:", len, buf);
    if (ep->out_cb)
        (*ep->out_cb)(ep, buf, len);
}

/*
 * process IN transaction on EP1, EP2, ...
 */
void usbdev_proto_ep_IN(struct usbdev_ep_t *ep)
{
//    DPRINT("c", 'i');

    /* EP0 - control endpoint */
    if ((ep->epn & 0x0f) == 0x00)
    {
        if (!setaddr)
            setaddr = 0;
        else
            usbdev_proto_send_IN(ep, 0);
    } else {
        if (ep->in.c)
            usbdev_proto_send_IN(ep, 0);
        else
            ;
//            os_event_raise(&ep->event, USBDEV_EVENT_MASK_EP_IN_READY);
    }
}

