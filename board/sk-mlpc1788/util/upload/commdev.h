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

#ifndef COMMDEV_H
#define COMMDEV_H

#include <libusb.h>
#include <types.h>

struct commdev_t {
    libusb_device_handle *handle;

};

void commdev_init();
void commdev_open(struct commdev_t *commdev, int idx);
void commdev_close(struct commdev_t *commdev);
int  commdev_send(struct commdev_t *commdev, uint8 *buf, int len, int timeout);
int  commdev_recv(struct commdev_t *commdev, uint8 *buf, int len, int timeout);
void commdev_exit();

#endif

