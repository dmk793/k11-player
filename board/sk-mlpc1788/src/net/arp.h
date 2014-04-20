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

#ifndef NET_ARP_H
#define NET_ARP_H

#include <types.h>
#include "net_def.h"

int arp_process(struct net_eth_frame_t *inf);
int arp_gratuitous(struct net_eth_frame_t *outf);
int arp_request(struct net_eth_frame_t *outf, uint32 dst_addr);
void arp_fill_cache(uint8 *haddr, uint32 paddr);
uint8* arp_get_haddr(uint32 paddr);

#endif

