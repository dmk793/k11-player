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
#include "arp.h"
#include "net.h"
#include "net_def.h"
#include <os.h>
#include <os.h>
#include "../osw_objects.h"

#define DEBUGARP

/* NOTE */
#ifndef DEBUGNET
    #undef DEBUGARP
#endif

#ifdef DEBUGARP
    #define DEBUGARP_STR(s) debug_str(s)
    #define DEBUGARP_HEX(n, l) debug_hex(n, l)
    #define DEBUGARP_HEX_WRAP(s, n, l) debug_hex_wrap(s, n, l)
#else
    #define DEBUGARP_STR(s)
    #define DEBUGARP_HEX(n, l)
    #define DEBUGARP_HEX_WRAP(s, n, l)
#endif

#pragma pack(push, 1)
struct arp_packet_t {
#define ARP_HTYPE_ETHERNET 0x0001
    uint16 htype;  /*  0,   0    Hardware type (HTYPE) */
    uint16 ptype;  /*  2,  16    Protocol type (PTYPE) */
    uint8  hlen;   /*  4,  32    Hardware address length (HLEN) */
    uint8  plen;   /*  5,  40    Protocol address length (PLEN)  */
#define ARP_OPER_REQUEST 0x0001
#define ARP_OPER_REPLY   0x0002
    uint16 oper;           /*  6,  48    Operation (OPER) */
    uint8  sha[HADDR_LEN]; /*  8,  64    Sender hardware address (SHA) */
    uint32 spa;            /* 14, 112    Sender protocol address (SPA) */
    uint8  tha[HADDR_LEN]; /* 18, 144    Target hardware address (THA) */
    uint32 tpa;            /* 24, 192    Target protocol address (TPA) */
};

struct arp_cache_t {
#define ARP_CACHE_SIZE  8
    int fp;
    struct {
        uint32 paddr;
        uint8  haddr[HADDR_LEN];
    } data[ARP_CACHE_SIZE];
};
#pragma pack(pop)

static struct arp_cache_t cache;

static int arp_haddr_match(uint8 *haddr1, uint8 *haddr2);

/*
 * ARGS
 *    inf    input frame
 *
 * RETURN
 *     count of bytes in response, 0 if there is no response
 */
int arp_process(struct net_eth_frame_t *inf)
{
    struct arp_packet_t *packet;
    uint32 paddr;

    packet  = (struct arp_packet_t *)inf->payload;

    /* update record in cache */
    arp_fill_cache(packet->sha, packet->spa);

    /* TODO fields sanity check */
    if (packet->oper != NET_HTONS(ARP_OPER_REQUEST))
        return 0;

    DEBUGARP_STR("=== arp request ===\r\n");

    memset(inf->head.mac_dst, 0xff, HADDR_LEN);           /* broadcast */
    memcpy(inf->head.mac_src, net_info.haddr, HADDR_LEN); /* self */

    packet->htype = NET_HTONS(ARP_HTYPE_ETHERNET);
    packet->ptype = NET_HTONS(NET_ETHERTYPE_IPV4);
    packet->hlen  = HADDR_LEN;
    packet->plen  = 4; /* NOTE IPv4 */
    packet->oper  = NET_HTONS(ARP_OPER_REPLY);

    memcpy(packet->tha, packet->sha, HADDR_LEN);
    memcpy(&packet->tpa, &packet->spa, 4);

    paddr = NET_HTONL(net_info.paddr);

    memcpy(packet->sha,  &net_info.haddr, HADDR_LEN);
    memcpy(&packet->spa, &paddr, 4);

    return (sizeof(struct arp_packet_t) + sizeof(struct net_eth_frame_head_t));
}

/*
 *
 */
int arp_gratuitous(struct net_eth_frame_t *outf)
{
    struct arp_packet_t *outp;
    uint32 paddr;

    DEBUGARP_STR("arp, make gratoitous\r\n");

    outp = (struct arp_packet_t *)outf->payload;

    outf->head.ethertype = NET_HTONS(NET_ETHERTYPE_ARP);
    memset(outf->head.mac_dst, 0xff, HADDR_LEN);
    memcpy(outf->head.mac_src, &net_info.haddr, HADDR_LEN);

    outp->htype = NET_HTONS(ARP_HTYPE_ETHERNET);
    outp->ptype = NET_HTONS(NET_ETHERTYPE_IPV4);
    outp->hlen  = HADDR_LEN;
    outp->plen  = 4; /* NOTE IPv4 */
    outp->oper  = NET_HTONS(ARP_OPER_REQUEST);

    paddr = NET_HTONL(net_info.paddr);

    memcpy(outp->sha,  &net_info.haddr, HADDR_LEN);
    memcpy(&outp->spa, &paddr, 4);

    memset(outp->tha, 0, HADDR_LEN);
    memcpy(&outp->tpa, &paddr, 4);

    return (sizeof(struct arp_packet_t) + sizeof(struct net_eth_frame_head_t));
}

/*
 *
 */
int arp_request(struct net_eth_frame_t *outf, uint32 dst_addr)
{
    struct arp_packet_t *outp;
    uint32 spa, tpa;

    /* broadcast packet */
    memset(outf->head.mac_dst, 0xff, HADDR_LEN);
    memcpy(outf->head.mac_src, &net_info.haddr, HADDR_LEN);

    outp = (struct arp_packet_t *)outf->payload;
    outf->head.ethertype = NET_HTONS(NET_ETHERTYPE_ARP);
    outp->htype          = NET_HTONS(ARP_HTYPE_ETHERNET);
    outp->ptype          = NET_HTONS(NET_ETHERTYPE_IPV4);
    outp->hlen           = HADDR_LEN;
    outp->plen           = 4; /* NOTE IPv4 */
    outp->oper           = NET_HTONS(ARP_OPER_REQUEST);

    spa = NET_HTONL(net_info.paddr);
    tpa = NET_HTONL(dst_addr);

    memcpy(outp->sha,  &net_info.haddr, HADDR_LEN);
    memcpy(&outp->spa, &spa, 4);

    memset(outp->tha, 0, HADDR_LEN);
    memcpy(&outp->tpa, &tpa, 4);

    return (sizeof(struct arp_packet_t) + sizeof(struct net_eth_frame_head_t));
}

/*
 * ARGS
 *     paddr    protocol address
 *
 * RETURN
 *     pointer to haddr or NULL if entry not found
 */
uint8* arp_get_haddr(uint32 paddr)
{
    int i;
    uint8* ret;

    os_mutex_lock(&net.mutex, NET_MUTEX_MASK_ARP, OS_FLAG_NONE, OS_WAIT_FOREVER);

    DEBUGARP_STR("arp cache, lookup entry ");
    DEBUGARP_HEX(&paddr, 4); DEBUGARP_STR("\r\n");

    ret = NULL;
    for (i = 0; i < ARP_CACHE_SIZE; i++)
    {
        DEBUGARP_STR("    entry ");
        DEBUGARP_HEX(&cache.data[i].paddr, 4); DEBUGARP_STR(", ");
        DEBUGARP_HEX(&cache.data[i].haddr, HADDR_LEN); DEBUGARP_STR("\r\n");

        if (cache.data[i].paddr == paddr)
        {
            DEBUGARP_STR("    found\r\n");
            ret = cache.data[i].haddr;
            goto out;
        }
    }

out:
    os_mutex_unlock(&net.mutex, NET_MUTEX_MASK_ARP);
    return ret;
}

/*
 * RETURN
 *     1 if hardware address match, 0 otherwise
 */
static int arp_haddr_match(uint8 *haddr1, uint8 *haddr2)
{
    uint8 i;
    uint8 m;

    m = 0;
    for (i = 0; i < HADDR_LEN; i++)
    {
        m ^= haddr1[i] ^ haddr2[i];
        if (m)
            return 0;
    }

    return 1;
}

/*
 * fill entry in ARP cache
 *
 * NOTE entries values are stored in big endian format
 *
 */
void arp_fill_cache(uint8 *haddr, uint32 paddr)
{
    int i;
    uint8 bcast[HADDR_LEN];

    os_mutex_lock(&net.mutex, NET_MUTEX_MASK_ARP, OS_FLAG_NONE, OS_WAIT_FOREVER);

    /* exclude broadcast message */
    memset(&bcast, 0xff, HADDR_LEN);
    if (arp_haddr_match(haddr, bcast))
        goto out;

    /* NOTE exclude self */
    if (paddr == net_info.paddr)
        goto out;

    for (i = 0; i < ARP_CACHE_SIZE; i++)
    {
        /* if entry was found update it */
        if (cache.data[i].paddr == paddr)
        {
            DEBUGARP_STR("arp cache, entry found ");
            DEBUGARP_HEX(haddr, HADDR_LEN); DEBUGARP_STR(", ");
            DEBUGARP_HEX(&paddr, 4); DEBUGARP_STR("\r\n");

            memcpy(cache.data[i].haddr, haddr, HADDR_LEN);
            goto out;
        }
    }

    /* entry not found */
    if (i == ARP_CACHE_SIZE)
    {
        DEBUGARP_STR("arp cache, new entry ");
        DEBUGARP_HEX(haddr, HADDR_LEN); DEBUGARP_STR(", ");
        DEBUGARP_HEX(&paddr, 4); DEBUGARP_STR("\r\n");

        cache.data[cache.fp].paddr = paddr;
        memcpy(cache.data[cache.fp].haddr, haddr, HADDR_LEN);

        cache.fp++;
        if (cache.fp >= ARP_CACHE_SIZE)
            cache.fp = 0;
    }
out:
    os_mutex_unlock(&net.mutex, NET_MUTEX_MASK_ARP);
}

