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
#include <stimer.h>
#include <debug.h>
#include "../eth.h"
#include "net.h"
#include "arp.h"
#include "udp.h"
#include "icmp.h"
#include "ipv4.h"
#include "../osw_objects.h"
#include "../upload.h"

#define OSW_NETQUEUE_LEN    4
#define OSW_NETQUEUE_MSIZE  (sizeof (struct net_eth_frame_t))

#define NET_ARP_SEND_TO    10000

struct net_t net;
struct net_info_t net_info;

static void net_arp_gratuitous();

/*
 *
 */
void net_task()
{
    struct net_eth_frame_t frame;
    BASE_TYPE len;
    struct os_multi_event_t *mevent;

    {
        dprint("sn", "Start net");

        net_info.haddr[0] = 0x00; /* NOTE low bits have special meaning */
        net_info.haddr[1] = 0xab;
        net_info.haddr[2] = 0x02;
        net_info.haddr[3] = 0xac;
        net_info.haddr[4] = 0x03;
        net_info.haddr[5] = 0xad;

        net_info.paddr = net_aton(NET_IPV4_ADDR);

        if (!eth_init())
        {
            dprint("sn", ERR_PREFIX "Net init error");
            goto error;
        }

        {
            uint8 *paddr;

            paddr = (uint8*)&net_info.paddr;
            dprint("s1d*.1d*.1d*.1dn", "IP  ", paddr[3], paddr[2], paddr[1], paddr[0]);
        }
    }
    net.qtx   = os_queue_init(OSW_NETQUEUE_LEN, OSW_NETQUEUE_MSIZE);
    net.qproc = os_queue_init(OSW_NETQUEUE_LEN, OSW_NETQUEUE_MSIZE);

    mevent = os_multi_init(2);
    os_multi_add_event(mevent, &net.events, NET_EVENT_MASK_NET_IRQ);
    os_multi_add_queue(mevent, net.qtx, OS_MULTI_QUEUE_NOT_EMPTY);

    os_event_raise(&net.events, NET_EVENT_MASK_INIT);

    stimer_settime(&net.arpto);
    while (1)
    {
        if (eth_linkup())
        {
            if (os_multi_wait(mevent, OS_MULTI_LOCK_OR, OS_MS2TICK(1000)) == OS_ERR_NONE)
            {
                if (os_event_wait(&net.events, NET_EVENT_MASK_NET_IRQ,
                            OS_FLAG_NOWAIT | OS_FLAG_CLEAR, 0) == OS_ERR_NONE)
                    eth_handler_bh();

                if (eth_txready())
                {
                    if (os_queue_remove(net.qtx, OS_FLAG_NOWAIT, 0 /* don't care */,
                                &frame, &len) == OS_ERR_NONE)
                    {
                        eth_send((uint8*)&frame, len);
                    }
                } else {
                    /* should not reached */
                    os_wait_ms(10);
                }
            }

            /* ARP gratuitous */
            if (stimer_deltatime(net.arpto) >= NET_ARP_SEND_TO)
            {
                stimer_settime(&net.arpto);
                net_arp_gratuitous();
            }
        } else {
            stimer_settime(&net.arpto);
            os_wait_ms(100);
        }
    }
error:
    os_enable_irq();
    for(;;)
        os_wait_ms(1000);
}

/*
 *
 */
static void net_arp_gratuitous()
{
    struct net_eth_frame_t frame;
    net_send(&frame, arp_gratuitous(&frame));
}

/*
 *
 */
void net_send(struct net_eth_frame_t *frame, int len)
{
    if (len)
        os_queue_add(net.qtx, OS_FLAG_NONE, OS_MS2TICK(100), frame, len);
}

/*
 *
 */
void net_receive(struct net_eth_frame_t *frame, int len)
{
    /* NOTE 802.1Q tag not handled */
    switch (NET_HTONS(frame->head.ethertype))
    {
        case NET_ETHERTYPE_ARP:
            net_send(frame, arp_process(frame));
            break;
        case NET_ETHERTYPE_IPV4:
            {
                uint8 haddr[HADDR_LEN];

                len = ipv4_process(frame, len);
                if (len == 0)
                    break;

                /* swap MAC addresses */
                memcpy(haddr, frame->head.mac_src, HADDR_LEN);
                memcpy(frame->head.mac_src, frame->head.mac_dst, HADDR_LEN);
                memcpy(frame->head.mac_dst, haddr, HADDR_LEN);

                if (len)
                    net_send(frame, len);
            }
            break;
    }
}

/*
 *
 */
void net_process_task()
{
    struct net_eth_frame_t frame; /* NOTE be care of task stack, frame is large enough (1536) */
    struct ipv4_packet_t *ipv4;
    int len;

    os_event_wait(&net.events, NET_EVENT_MASK_INIT, OS_FLAG_NONE, OS_WAIT_FOREVER);
    while (1)
    {
        os_queue_remove(net.qproc, OS_FLAG_NONE, OS_WAIT_FOREVER, &frame, &len);

        switch (NET_HTONS(frame.head.ethertype))
        {
            case NET_ETHERTYPE_IPV4:
                {
                    ipv4 = (struct ipv4_packet_t *)frame.payload;
                    if (ipv4->head.protocol == IPV4_PROTOCOL_UDP)
                        udp_process(ipv4);
                }
                break;
            default:
                dprint("sn", WARN_PREFIX "net_process, unknown frame");
        }
    }
}

/* =======================================================
 * = UDP suite
 * =======================================================
 */
#define UDP_CALLBACKS_COUNT   2
struct udp_cb_t {
    pudp_cb cb;
    uint16 port;
};

struct udp_cb_t udp_callback[UDP_CALLBACKS_COUNT];

/*
 * Add callback function for receive packet from specific UDP port
 */
void net_udp_add_cb(uint16 port, pudp_cb cb)
{
    int i;

    os_mutex_lock(&net.mutex, NET_MUTEX_MASK_UDP, OS_FLAG_NONE, OS_WAIT_FOREVER);
    for (i = 0; i < UDP_CALLBACKS_COUNT; i++)
    {
        if (udp_callback[i].cb == NULL)
        {
            udp_callback[i].cb   = cb;
            udp_callback[i].port = port;
            break;
        }
    }
    os_mutex_unlock_ns(&net.mutex, NET_MUTEX_MASK_UDP);
}

/*
 *
 */
void net_udp_delete_cb(pudp_cb cb)
{
    int i;

    os_mutex_lock(&net.mutex, NET_MUTEX_MASK_UDP, OS_FLAG_NONE, OS_WAIT_FOREVER);
    for (i = 0; i < UDP_CALLBACKS_COUNT; i++)
    {
        if (udp_callback[i].cb == cb)
        {
            udp_callback[i].cb = NULL;
            break;
        }
    }
    os_mutex_unlock_ns(&net.mutex, NET_MUTEX_MASK_UDP);
}

/*
 * Called by net suite (ipv4_process()->udp_process()) when UDP packet arrived.
 *
 * ARGS
 *     src_ip      source IP address
 *     dst_ip      destination IP address
 *     src_port    source port
 *     dst_port    destination port
 *     payload     payload
 *     length      length of payload
 */
void net_udp_receive(uint32 src_ip, uint32 dst_ip, uint16 src_port, uint16 dst_port, uint8 *payload, uint16 length)
{
    int i;

    os_mutex_lock(&net.mutex, NET_MUTEX_MASK_UDP, OS_FLAG_NONE, OS_WAIT_FOREVER);
    for (i = 0; i < UDP_CALLBACKS_COUNT; i++)
    {
        if (udp_callback[i].cb != NULL && udp_callback[i].port == dst_port)
            (*udp_callback[i].cb)(src_ip, dst_ip, src_port, dst_port, payload, length);
    }
    os_mutex_unlock_ns(&net.mutex, NET_MUTEX_MASK_UDP);

//    {
//        int i;
//        debug_str("udp rx packet:\r\n");
//        debug_hex_wrap("src_ip   = ", &src_ip, 4);
//        debug_hex_wrap("dst_ip   = ", &dst_ip, 4);
//        debug_hex_wrap("src_port = ", &src_port, 2);
//        debug_hex_wrap("dst_port = ", &dst_port, 2);
//        debug_str("payload, length "); debug_hex(&length, 2);
//        for (i = 0; i < length; i++)
//        {
//            if ((i % 16) == 0)
//                debug_str("\r\n    ");
//            debug_hex(&payload[i], 1); debug_str(" ");
//        }
//        debug_str("\r\n");
//    }

//    payload[1] = 0xef;
//    payload[2]++;
//    net_udp_send(net_info.paddr,
//            (192 << 24) | (168 << 16) | (1 << 8) | (10 << 0),
//            9930, 9930, payload, 1000);

//    if (dst_port == 9930)
//        upload_receive(src_ip, dst_ip, src_port, dst_port, payload, length);
}

/*
 * Send
 *
 * ARGS
 *     src_ip      source IP address
 *     dst_ip      destination IP address
 *     src_port    source port
 *     dst_port    destination port
 *     payload     payload
 *     length      length of payload
 *
 */
#define NET_UDP_STATE_IDLE       0
#define NET_UDP_STATE_GET_ARP    1

void net_udp_send(uint32 src_ip, uint32 dst_ip, uint16 src_port, uint16 dst_port, uint8 *payload, uint16 length)
{
    struct net_eth_frame_t frame; /* NOTE be care of task stack, frame is large enough (1536) */
    struct ipv4_packet_t *ipv4;
    struct udp_packet_t *udp;
    uint8 *dst_haddr;
    int arptry;

    /* TODO broadcast packets support */
    arptry = 0;
    dst_haddr = NULL;

    /* check distinaiton MAC */
    while (1)
    {
        dst_haddr = arp_get_haddr(NET_HTONL(dst_ip));
        if (dst_haddr == NULL)
        {
            net_send(&frame, arp_request(&frame, dst_ip));
            arptry++;
            os_wait_ms(50);
        } else {
            break;
        }

        if (arptry >= 2)
            return;
    }
//    debug_str("arp done\r\n");

    ipv4 = (struct ipv4_packet_t *)frame.payload;
    ipv4->head.ihl        = 5;

    /* fill UDP packet */
    udp = (struct udp_packet_t*)ipv4_payload(ipv4);
    udp->head.src_port = NET_HTONS(src_port);
    udp->head.dst_port = NET_HTONS(dst_port);
    length += sizeof(struct udp_packet_head_t); /* NOTE */
    udp->head.length   = NET_HTONS(length);
    memcpy(udp->payload, payload, length); /* TODO length sanity check */

    length += IPV4_HEAD_LEN(ipv4); /* NOTE */

    /* fill IPv4 fields */
    ipv4->head.version      = IPV4_VERSION;
    ipv4->head.dscp         = 0x21;
    ipv4->head.ecn          = 0x00;
    ipv4->head.total_len    = NET_HTONS(length);
    ipv4->head.ident        = NET_HTONS(net.udp_ident++);
    ipv4->head.f_offset_msb = 0x00;
    ipv4->head.f_offset_lsb = 0x00;
    ipv4->head.flags        = IPV4_FLAG_DF;
    ipv4->head.ttl          = 64;
    ipv4->head.protocol     = IPV4_PROTOCOL_UDP;
    ipv4->head.src_ip       = NET_HTONL(src_ip);
    ipv4->head.dst_ip       = NET_HTONL(dst_ip);
    ipv4->head.head_cs      = ipv4_head_cs(ipv4);

    /* checksum of UDP calculated from UDP pseudo header, so fill it after IPv4 packet */
    udp->head.cs = udp_cs(ipv4);

    /* fill ethernet frame fields */
    memcpy(frame.head.mac_src, net_info.haddr, HADDR_LEN);
    memcpy(frame.head.mac_dst, dst_haddr, HADDR_LEN);
    frame.head.ethertype = NET_HTONS(NET_ETHERTYPE_IPV4);

    net_send(&frame, (length + sizeof(struct net_eth_frame_head_t)));
}

/*
 * Convert string representation of IPv4 address to format used by this
 * network stack implementation. For example "192.168.1.100" will produce
 * ((192 << 24) | (168 << 16) | (1 << 8) | (100 << 0)) number.
 *
 * RETURN
 *     resulting number that represent IP address or zero if parser error was occured.
 */
uint32 net_aton(char *in)
{
    uint32 result;
#define STEM_LENGTH   4 /* length of one member plus null terminator */
    char stem[STEM_LENGTH];
    char *p;

    if (!in)
        return 0;

    result = 0;

    memset(stem, 0, STEM_LENGTH);
    p = stem;
    for (;; in++)
    {
        if (*in == '.' || *in == 0)
        {
            int div;
            int len;

            len = strlen(stem);

            *p = 0;
            if (len < 1 || len > 3)
                return 0;

            result <<= 8;
            div = 1;
            len--;
            while (len--)
                div *= 10;

            p = stem;
            while (div)
            {
                result += ((*p++) - '0') * div;
                div /= 10;
            }
            memset(stem, 0, STEM_LENGTH);
            p = stem;

            if (*in)
                continue;
            else
                break;
        }

        /* length of number exceed */
        if (p == &stem[STEM_LENGTH - 1])
            return 0;

        if (*in >= '0' && *in <= '9')
            *p++ = *in;
        else
            return 0;
    }

    return result;
}

