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

#ifndef ETH_H
#define ETH_H

#include <types.h>

struct ethernet_t {
    uint8 phyaddr;

    uint8 speed;
    uint8 duplex;

    uint8 link;

    uint32 anretry;

    uint8 macaddr[6];

    uint32 to;
#define ETHERNET_STATE_IDLE             0x00
#define ETHERNET_STATE_AUTOAN           0x01
    uint8 state;
};

extern struct ethernet_t ethernet;

int eth_init();
int eth_linkup();
int eth_txready();
void eth_handler_bh();
void eth_send(uint8 *packet, uint32 len);

void Ethernet_Handler();

#endif

