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
#include <string.h>
#include "net.h"
#include "net_def.h"
#include "udp.h"

#define DEBUGUDP

/* NOTE */
#ifndef DEBUGNET
    #undef DEBUGUDP
#endif

#ifdef DEBUGUDP
    #define DEBUGUDP_STR(s) debug_str(s)
    #define DEBUGUDP_HEX(n, l) debug_hex(n, l)
    #define DEBUGUDP_HEX_WRAP(s, n, l) debug_hex_wrap(s, n, l)
#else
    #define DEBUGUDP_STR(s)
    #define DEBUGUDP_HEX(n, l)
    #define DEBUGUDP_HEX_WRAP(s, n, l)
#endif

/*
 * ARGS
 *    packet    input packet
 *
 * RETURN
 *     count of bytes in response (size of udp packet), 0 if there is no response
 */
int udp_process(struct ipv4_packet_t *ipv4)
{
    struct udp_packet_t *udp;

    DEBUGUDP_STR("=== udp process ===\r\n");

    if (ipv4->head.ihl > 5)
        udp = (struct udp_packet_t *)&ipv4->payload[
            (ipv4->head.ihl * 4) - sizeof(struct ipv4_head_t)];
    else
        udp = (struct udp_packet_t *)ipv4->payload;

#ifdef DEBUGUDP
    {
        DEBUGUDP_HEX_WRAP("src port = ", &udp->head.src_port, 2);
        DEBUGUDP_HEX_WRAP("dst port = ", &udp->head.dst_port, 2);
        DEBUGUDP_HEX_WRAP("length   = ", &udp->head.length, 2);
        DEBUGUDP_HEX_WRAP("cs       = ", &udp->head.cs, 2);
    }
#endif

    /* XXX discard packet longer then 1008 bytes */
    if (NET_HTONS(udp->head.length) > 1008)
        return 0;

    /* check CS if it not zero */
    if (udp->head.cs)
    {
        if (udp_cs(ipv4) != udp->head.cs)
        {
            DEBUGUDP_STR("checksum mismatch\r\n");
        }
    }


    net_udp_receive(
            NET_HTONL(ipv4->head.src_ip),
            NET_HTONL(ipv4->head.dst_ip),
            NET_HTONS(udp->head.src_port),
            NET_HTONS(udp->head.dst_port),
            udp->payload,
            NET_HTONS(udp->head.length) - sizeof(struct udp_packet_head_t));

//    {
//        uint16 len;
//        uint16 port;
//#define BUF_SIZE 3
//        uint8 buf[BUF_SIZE];
//
//        buf[0] = 0x00;
//        buf[1] = 0xab;
//        buf[2] = 0xac;
//
//        len = (sizeof(struct udp_packet_head_t) + BUF_SIZE);
//        memcpy(udp->payload, buf, BUF_SIZE);
//
////        len = (sizeof(struct udp_packet_head_t) + 1000);
//
//        port               = udp->head.src_port;
//        udp->head.src_port = udp->head.dst_port;
//        udp->head.dst_port = port;
//
//        ipv4->head.total_len = NET_HTONS(IPV4_HEAD_LEN(ipv4) + len);
//
//        udp->head.length   = NET_HTONS(len);
//
//
////        ipv4->head.total_len = NET_HTONS(len);
//        udp->head.cs       = udp_cs(ipv4);
//
//        return len;
//    }

    return 0;
}

/*
 *
 */
uint16 udp_cs(struct ipv4_packet_t *ipv4)
{
    uint32 cs;
    uint32 dlen;
    uint16 *data;

    struct udp_packet_t *udp;
    if (ipv4->head.ihl > 5)
        udp = (struct udp_packet_t *)&ipv4->payload[
            IPV4_HEAD_LEN(ipv4) - sizeof(struct ipv4_head_t)];
    else
        udp = (struct udp_packet_t *)ipv4->payload;

    dlen = (NET_HTONS(ipv4->head.total_len) - IPV4_HEAD_LEN(ipv4));

    cs = (ipv4->head.src_ip & 0xffff) +
         (ipv4->head.src_ip >> 16) +
         (ipv4->head.dst_ip & 0xffff) +
         (ipv4->head.dst_ip >> 16) +
         (ipv4->head.protocol << 8) +
         NET_HTONS(dlen) +
         udp->head.src_port + 
         udp->head.dst_port +
         udp->head.length;

    dlen -= sizeof(struct udp_packet_head_t);
    data = (uint16*)udp->payload;

    while (dlen > 1)
    {
        cs += *data++;
        dlen -= 2;
    }
    if (dlen)
        cs += (*data) & 0xff;

    cs = ~(((cs >> 16) + cs) & 0xffff);
    if (cs == 0)
        cs = 0xffff;

    return cs;
}

