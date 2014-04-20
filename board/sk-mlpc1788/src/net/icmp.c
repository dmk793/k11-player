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

#include <string.h>
#include <debug.h>
#include "icmp.h"
#include "net.h"
#include "net_def.h"

#define DEBUGICMP

/* NOTE */
#ifndef DEBUGNET
    #undef DEBUGICMP
#endif

#ifdef DEBUGICMP
    #define DEBUGICMP_STR(s) debug_str(s)
    #define DEBUGICMP_HEX(n, l) debug_hex(n, l)
    #define DEBUGICMP_HEX_WRAP(s, n, l) debug_hex_wrap(s, n, l)
#else
    #define DEBUGICMP_STR(s)
    #define DEBUGICMP_HEX(n, l)
    #define DEBUGICMP_HEX_WRAP(s, n, l)
#endif

static uint16 icmp_cs(struct icmp_packet_t *icmp, int len);

/*
 * ARGS
 *    icmp    input packet
 *    len     size of input packet
 *
 * RETURN
 *     count of bytes in response (size of udp icmp packet), 0 if there is no response
 */
int icmp_process(struct icmp_packet_t *icmp, int len)
{
    /* XXX simple and ugly implementation, PING only */

    DEBUGICMP_STR("=== icmp request ===\r\n");

#ifdef DEBUGICMP
    {
        int i;
        uint8 *buf;

        i = len;
        buf = (uint8*)icmp;
        for (i = 0; i < len; i++)
        {
            DEBUGICMP_HEX(&buf[i], 1); DEBUGICMP_STR(" ");
            if ((i % 16) == 0)
                DEBUGICMP_STR("\r\n");
        }
        DEBUGICMP_STR("\r\n");
    }
#endif

    if (icmp_cs(icmp, len) != icmp->head.cs)
    {
        DEBUGICMP_STR("cs mismatch\r\n");

        return 0;
    }

    if (icmp->head.type == 8 && icmp->head.code == 0)
    {
        DEBUGICMP_STR("echo request\r\n");
    }

    icmp->head.type = 0;
    icmp->head.cs = icmp_cs(icmp, len);

    return len;
}

/*
 *
 */
static uint16 icmp_cs(struct icmp_packet_t *icmp, int len)
{
    uint32 cs;
    uint16 *buf;

    cs = 0;
    buf = (uint16*)icmp;
    while (len > 1)
    {
        cs += *buf++;
        len -= 2;
    }

    if (len)
        cs += (*buf) & 0xff;

    cs -= icmp->head.cs;

    return ~(((cs >> 16) + cs) & 0xffff);
}


