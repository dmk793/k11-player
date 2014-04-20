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

#ifndef USBDEV_PROTO_H
#define USBDEV_PROTO_H

#include <types.h>

#pragma pack(push, 1)

/*
 * device requests
 */
struct dev_req_t {
    uint8  bmRequestType;
    uint8  bRequest;
    uint16 wValue;
    uint16 wIndex;
    uint16 wLength;
};

/******************************************************************************
 * Standard request codes (table 9-4)
 *****************************************************************************/
#define GET_DESCRIPTOR    6
#define SET_ADDRESS       5
#define SET_CONFIGURATION 9
/******************************************************************************
 * Descriptor Types (table 9-5)
 *****************************************************************************/
#define DESC_TYPE_DEVICE                    0x01
#define DESC_TYPE_CONFIGURATION             0x02
#define DESC_TYPE_STRING                    0x03
#define DESC_TYPE_INTERFACE                 0x04
#define DESC_TYPE_ENDPOINT                  0x05
#define DESC_TYPE_DEVICE_QUALIFIER          0x06
#define DESC_TYPE_OTHER_SPEED_CONFIGURATION 0x07
#define DESC_TYPE_INTERFACE_POWER           0x08
/******************************************************************************
 * USB Device Descriptor (table 9-8)
 *****************************************************************************/
struct USB_Device_Descriptor {
    uint8  bLength;
    uint8  bDescriptorType;
    uint16 bcdUSB;
    uint8  bDeviceClass;
    uint8  bDeviceSubClass;
    uint8  bDeviceProtocol;
    uint8  bMaxPacketSize0;
    uint16 idVendor;
    uint16 idProduct;
    uint16 bcdDevice;
    uint8  iManufacturer;
    uint8  iProduct;
    uint8  iSerialNumber;
    uint8  bNumConfigurations;
};
/******************************************************************************
 * Device_Qualifier (table 9-9)
 *****************************************************************************/
struct USB_Device_Qualifier {
    uint8  bLength;
    uint8  bDescriptorType;
    uint16 bcdUSB;
    uint8  bDeviceClass;
    uint8  bDeviceSubClass;
    uint8  bDeviceProtocol;
    uint8  bMaxPacketSize0;
    uint8  bNumConfigurations;
    uint8  reserved;
};
/******************************************************************************
 * Configuration Descriptor (table 9-10)
 *****************************************************************************/
struct USB_Configuration_Descriptor {
    uint8  bLength;
    uint8  bDescriptorType;
    uint16 wTotalLength;
    uint8  bNumInterfaces;
    uint8  bConfigurationValue;
    uint8  iConfiguration;
    uint8  bmAttributes;
    uint8  bMaxPower; 
};

#pragma pack(pop)

void usbdev_proto_control_ep_OUT(uint8 *buf, int len);
void usbdev_proto_ep_OUT(struct usbdev_ep_t *ep, uint8 *buf, int len);
void usbdev_proto_ep_IN(struct usbdev_ep_t *ep);

#define SET_STALL   1
void usbdev_proto_send_IN(struct usbdev_ep_t *ep, uint8 stall);

#endif

