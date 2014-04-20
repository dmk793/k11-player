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

#ifndef NET_UDP_H
#define NET_UDP_H

#include <types.h>
#include "ipv4.h"

#pragma pack(push, 1)
struct udp_packet_head_t {
    uint16 src_port;
    uint16 dst_port;
    uint16 length;
    uint16 cs;
};

struct udp_packet_t {
    struct udp_packet_head_t head;
#define UDP_MAX_PAYLOAD_SIZE   65507 /* XXX */
    uint8 payload[UDP_MAX_PAYLOAD_SIZE];
};

///* pseudo packet for UDP framework */
//struct udp_packet2_t {
//    uint32 src_ip;
//    uint32 dst_ip;
//    uint16 src_port;
//    uint16 dst_port;
//    uint16 length;
//#define UDP_MAX_PAYLOAD_SIZE2 1500 /* XXX */
//    uint8 payload[UDP_MAX_PAYLOAD_SIZE2];
//};
#pragma pack(pop)

int udp_process(struct ipv4_packet_t *ipv4);
uint16 udp_cs(struct ipv4_packet_t *ipv4);

#endif

