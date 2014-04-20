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

#include <debug.h>
#include "ipv4.h"
#include "udp.h"
#include "net.h"
#include "arp.h"
#include "icmp.h"
#include <os.h>
#include "../osw_objects.h"

#define DEBUGIPV4

/* NOTE */
#ifndef DEBUGNET
    #undef DEBUGIPV4
#endif

#ifdef DEBUGIPV4
    #define DEBUGIPV4_STR(s) debug_str(s)
    #define DEBUGIPV4_HEX(n, l) debug_hex(n, l)
    #define DEBUGIPV4_HEX_WRAP(s, n, l) debug_hex_wrap(s, n, l)
#else
    #define DEBUGIPV4_STR(s)
    #define DEBUGIPV4_HEX(n, l)
    #define DEBUGIPV4_HEX_WRAP(s, n, l)
#endif

/*
 * ARGS
 *    inf    input frame
 *
 * RETURN
 *     count of bytes in response, 0 if there is no response
 */
int ipv4_process(struct net_eth_frame_t *inf, int flen)
{
    struct ipv4_packet_t *ipv4;

    ipv4 = (struct ipv4_packet_t *)inf->payload;

    DEBUGIPV4_STR("=== ipv4 process === \r\n");

#ifdef DEBUGIPV4
    {
        uint8 version;
        uint8 ihl;
        uint8 dscp;
        uint8 ecn;
        uint8 flags;
        uint16 f_offset_msb;
        uint16 f_offset_lsb;

        version      = ipv4->head.version;
        ihl          = ipv4->head.ihl;
        dscp         = ipv4->head.dscp;
        ecn          = ipv4->head.ecn;
        flags        = ipv4->head.flags;
        f_offset_msb = ipv4->head.f_offset_msb;
        f_offset_lsb = ipv4->head.f_offset_lsb;
        DEBUGIPV4_HEX_WRAP("version       = ", &version, 1);
        DEBUGIPV4_HEX_WRAP("ihl           = ", &ihl, 1);
        DEBUGIPV4_HEX_WRAP("dscp          = ", &dscp, 1);
        DEBUGIPV4_HEX_WRAP("ecn           = ", &ecn, 1);
        DEBUGIPV4_HEX_WRAP("total_len     = ", &ipv4->head.total_len, 2);
        DEBUGIPV4_HEX_WRAP("ident         = ", &ipv4->head.ident, 2);
        DEBUGIPV4_HEX_WRAP("flags         = ", &flags, 1);
        DEBUGIPV4_HEX_WRAP("f_offset_msb  = ", &f_offset_msb, 2);
        DEBUGIPV4_HEX_WRAP("f_offset_lsb  = ", &f_offset_lsb, 2);
        DEBUGIPV4_HEX_WRAP("ttl           = ", &ipv4->head.ttl, 1);
        DEBUGIPV4_HEX_WRAP("protocol      = ", &ipv4->head.protocol, 1);
        DEBUGIPV4_HEX_WRAP("checksum      = ", &ipv4->head.head_cs, 2);
        DEBUGIPV4_HEX_WRAP("src IP        = ", &ipv4->head.src_ip, 4);
        DEBUGIPV4_HEX_WRAP("dst IP        = ", &ipv4->head.dst_ip, 4);
//        if (ihl > 5)
//            DEBUGIPV4_HEX_WRAP("options   = ", &ipv4->options, 4);
    }
#endif
    /* support only IPV4 */
    if (ipv4->head.version != IPV4_VERSION)
    {
        DEBUGIPV4_STR("no ipv4 version found\r\n");
        return 0;
    }

    /* IHL should be >= 5 */
    if (ipv4->head.ihl < 5)
    {
        DEBUGIPV4_STR("ihl too short\r\n");
        return 0;
    }

    /* catch only broadcast and self packets */
    if (NET_HTONL(ipv4->head.dst_ip) != net_info.paddr)
    {
        DEBUGIPV4_STR("address mismatch\r\n");
        return 0;
    }

    if (ipv4_head_cs(ipv4) != ipv4->head.head_cs)
    {
        DEBUGIPV4_STR("checksum mismatch\r\n");
        return 0;
    }

    /* NOTE fill ARP cache entry */
    arp_fill_cache(inf->head.mac_src, ipv4->head.src_ip);

    switch (ipv4->head.protocol)
    {
        case IPV4_PROTOCOL_UDP:
            {
//                uint16 len;
                /* handle this frame separetely */
                os_queue_add(net.qproc, OS_FLAG_NONE, OS_MS2TICK(100), inf, flen);

//                udp_process(ipv4);
//                len = udp_process(ipv4);
//                if (len)
//                {
//                    uint32 addr;
//
//                    addr = ipv4->head.src_ip;
//
//                    len += IPV4_HEAD_LEN(ipv4);
//
//                    ipv4->head.src_ip    = ipv4->head.dst_ip;
//                    ipv4->head.dst_ip    = addr;
//                    /* NOTE total_len already modified in UDP */
////                    ipv4->head.total_len = NET_HTONS(len);
//                    ipv4->head.head_cs   = ipv4_head_cs(ipv4);
//
//                    return (len + sizeof(struct net_eth_frame_head_t));
//                }
            }
            break;
        case IPV4_PROTOCOL_ICMP:
            {
                uint16 len;

                len = icmp_process((struct icmp_packet_t*)ipv4_payload(ipv4),
                        NET_HTONS(ipv4->head.total_len) - IPV4_HEAD_LEN(ipv4));
                if (len)
                {
                    uint32 addr;

                    addr = ipv4->head.src_ip;

                    len += IPV4_HEAD_LEN(ipv4);

                    ipv4->head.src_ip    = ipv4->head.dst_ip;
                    ipv4->head.dst_ip    = addr;
                    ipv4->head.total_len = NET_HTONS(len);
                    ipv4->head.head_cs   = ipv4_head_cs(ipv4);

                    return (len + sizeof(struct net_eth_frame_head_t));
                }
            }
            break;
        default:
            DEBUGIPV4_STR("unknown protocol\r\n");
            return 0;
    }

    return 0;
}

/*
 *
 */
uint16 ipv4_head_cs(struct ipv4_packet_t *ipv4)
{
    int i;
    uint32 cs;
    uint16 *buf;

    cs = 0;
    buf = (uint16*)ipv4;
    for (i = 0; i < ((ipv4->head.ihl * 4) / 2); i++)
        cs += buf[i];

    cs -= ipv4->head.head_cs;
    cs = ~(((cs >> 16) + cs)) & 0xffff;

    return cs;
}

/*
 * return pointer to payload of ipv4 packet
 */
uint8* ipv4_payload(struct ipv4_packet_t *ipv4)
{
    if (ipv4->head.ihl > 5)
        return &ipv4->payload[(ipv4->head.ihl * 4) - sizeof(struct ipv4_head_t)];
    else
        return ipv4->payload;
}

