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

#ifndef NET_IPV4_H
#define NET_IPV4_H

#include <types.h>
#include "net_def.h"

#pragma pack(push, 1)
struct ipv4_head_t {
    struct {
#define IPV4_VERSION    4
        uint8 ihl     : 4;
        uint8 version : 4;
    };
    struct {
        uint8 ecn  : 2;
        uint8 dscp : 6;
    };
    uint16 total_len;
    uint16 ident;
    struct {
#define IPV4_FLAG_RESERVED   (1 << 0)
#define IPV4_FLAG_DF         (1 << 1)
#define IPV4_FLAG_MF         (1 << 2)
        uint16 f_offset_msb : 5;
        uint16 flags        : 3;
        uint16 f_offset_lsb : 8;
    };
    uint8  ttl;
#define IPV4_PROTOCOL_ICMP   0x01
#define IPV4_PROTOCOL_UDP    0x11
    uint8  protocol;
    uint16 head_cs;
    uint32 src_ip;
    uint32 dst_ip;
};

struct ipv4_packet_t {
    struct ipv4_head_t head;
    union {
#define IPV4_MAX_OPTIONS_COUNT 4 /* XXX */
        uint32 options[IPV4_MAX_OPTIONS_COUNT];
#define IPV4_MAX_PAYLOAD_SIZE   65535 /* XXX */
        uint8 payload[IPV4_MAX_PAYLOAD_SIZE];
    };
};
#pragma pack(pop)

#define IPV4_HEAD_LEN(ipv4) \
    (ipv4->head.ihl * 4)

int ipv4_process(struct net_eth_frame_t *inf, int flen);
uint8* ipv4_payload(struct ipv4_packet_t *ipv4);
uint16 ipv4_head_cs(struct ipv4_packet_t *ipv4);

#endif

