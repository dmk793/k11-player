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

#ifndef NET_DEF_H
#define NET_DEF_H

#include <types.h>
#include "../eth_def.h"

#pragma pack(push, 1)

#define HADDR_LEN    6

struct net_eth_frame_head_t {
    uint8 mac_dst[HADDR_LEN];
    uint8 mac_src[HADDR_LEN];
#define NET_ETHERTYPE_ARP    0x0806
#define NET_ETHERTYPE_IPV4   0x0800
    uint16 ethertype;
};

struct net_eth_frame_t {
    struct net_eth_frame_head_t head;
#define MAX_PAYLOAD_SIZE     (EMAC_BLOCK_SIZE - sizeof(struct net_eth_frame_head_t))
    uint8 payload[MAX_PAYLOAD_SIZE];
};
#pragma pack(pop)

struct net_info_t {
    uint8  haddr[HADDR_LEN];  /* mac address */
    uint32 paddr;    /* IPv4 protocol address */ 
};

extern struct net_info_t net_info;

uint16 NET_HTONS(uint16 n16);
uint32 NET_HTONL(uint32 n32);

#endif

