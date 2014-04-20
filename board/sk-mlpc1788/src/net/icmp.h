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

#ifndef NET_ICMP_H
#define NET_ICMP_H

#include <types.h>
#include "ipv4.h"

#pragma pack(push, 1)
struct icmp_head_t {
    uint8 type;
    uint8 code;
    uint16 cs;
    uint32 hdata;
};

struct icmp_packet_t {
    struct icmp_head_t head;
#define ICMP_PAYLOD_MAX_SIZE 64 /* XXX */
    uint8 payload[ICMP_PAYLOD_MAX_SIZE];
};
#pragma pack(pop)

int icmp_process(struct icmp_packet_t *icmp, int len);


#endif

