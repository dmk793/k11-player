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
#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include "commdev.h"
#include "errcode.h"

#define COMMDEV_DEBUG

#ifdef COMMDEV_DEBUG
    #define COMMDEV_DEBUGF(...) printf(__VA_ARGS__)
#else
    #define COMMDEV_DEBUGF(...)
#endif

#define DEVICE_VID 0xa598
#define DEVICE_PID 0x0002

#define COMM_ENDPOINT   0x02

libusb_context *context = NULL;

/*
 *
 */
void commdev_init()
{
    int errcode;
    const struct libusb_version *lv;

    errcode = 0;

    lv = libusb_get_version();
    printf("libusb version %d.%d.%d\r\n", lv->major, lv->minor, lv->micro);

    if (libusb_init(&context))
    {
        fprintf(stderr, "libusb init error\r\n");
        errcode = ERRCODE_LIBUSB_INIT;
        context = NULL;
        goto error;
    }

    return;
error:
    exit(errcode);
}

/*
 * open USB device
 *
 * ARGS
 *     commdev    pointer to commdev_t structure
 *     idx        device index to open
 */
void commdev_open(struct commdev_t *commdev, int idx)
{
    int i, cnt, ndev;
    int errcode;
    libusb_device **devs;
    libusb_device *dev;
    int r;
    struct libusb_device_descriptor desc;

    cnt = libusb_get_device_list(context, &devs);
    if (cnt < 0)
    {
        fprintf(stderr, "can not get divice list\r\n");
        errcode = ERRCODE_LIBUSB_GETDEVLIST;
        goto error;
    }

    dev  = NULL;
    ndev = 0;
    for (i = 0; i < cnt; i++)
    {
        r = libusb_get_device_descriptor(devs[i], &desc);
        if (r < 0) {
            fprintf(stderr, "failed to get device descriptor");
            errcode = ERRCODE_LIBUSB_GETDEVDESC;
            goto error;
        }

        if (desc.idVendor == DEVICE_VID && desc.idProduct == DEVICE_PID)
        {
            printf("%04x:%04x (bus %d, device %d)\n",
                    desc.idVendor, desc.idProduct,
                    libusb_get_bus_number(devs[i]), libusb_get_device_address(devs[i]));
            dev = devs[i];
            if (ndev == idx)
                break;
            ndev++;
        }
    }

    if (dev == NULL)
    {
        fprintf(stderr, "can not find device\r\n");
        errcode = ERRCODE_LIBUSB_FINDDEV;
        goto error;
    }

    if (libusb_open(dev, &commdev->handle))
    {
        fprintf(stderr, "can not open device handle\r\n");
        errcode = ERRCODE_LIBUSB_OPEN;
        goto error;
    }

    if (libusb_claim_interface(commdev->handle, 0))
    {
        fprintf(stderr, "claim error\r\n");
        errcode = ERRCODE_LIBUSB_CLAIMINTERFACE;
        goto error;
    }

    libusb_free_device_list(devs, 1);

    /* XXX receive possible unhandled data */
    {
        uint8 buf[64];
        int n;

        libusb_bulk_transfer(commdev->handle, LIBUSB_ENDPOINT_IN | 0x01,
            buf, 64, &n, 100);
    }

    return;
error:
    exit(errcode);
}

/*
 *
 */
void commdev_close(struct commdev_t *commdev)
{
    libusb_release_interface(commdev->handle, 0);
    libusb_close(commdev->handle);
}

/*
 * RETURN
 *     count of bytes sended
 */
int commdev_send(struct commdev_t *commdev, uint8 *buf, int len, int timeout)
{
    int ret;
    int n;

    ret = libusb_bulk_transfer(commdev->handle, LIBUSB_ENDPOINT_OUT | COMM_ENDPOINT,
        buf, len, &n, timeout);
    if (ret)         
    {
        fprintf(stderr, "transfer (OUT) error %d\r\n", ret);
        exit(ERRCODE_USB_SEND); 
    }

#ifdef COMMDEV_DEBUG
    {
        int i;

        COMMDEV_DEBUGF("OUT: ");
        for (i = 0; i < n; i++)
        {
            if ((i % 16 == 0))
                COMMDEV_DEBUGF("\n     ");
            COMMDEV_DEBUGF("%02hhX ", buf[i]);
        }
        COMMDEV_DEBUGF("\n");
    }
#endif

    return n;
}

/*
 *
 */
int commdev_recv(struct commdev_t *commdev, uint8 *buf, int len, int timeout)
{
    int ret;
    int n;

    ret = libusb_bulk_transfer(commdev->handle, LIBUSB_ENDPOINT_IN | COMM_ENDPOINT,
        buf, len, &n, timeout);
    if (ret)         
    {
        fprintf(stderr, "transfer (IN) error %d\r\n", ret);
        exit(ERRCODE_USB_RECV); 
    }

#ifdef COMMDEV_DEBUG
    {
        int i;

        COMMDEV_DEBUGF(" IN: ");
        for (i = 0; i < n; i++)
        {
            if ((i % 16 == 0))
                COMMDEV_DEBUGF("\n     ");
            COMMDEV_DEBUGF("%02hhX ", buf[i]);
        }
        COMMDEV_DEBUGF("\n");
    }
#endif

    return n;
}

/*
 *
 */
void commdev_exit()
{
    if (context != NULL)
        libusb_exit(context);
}

