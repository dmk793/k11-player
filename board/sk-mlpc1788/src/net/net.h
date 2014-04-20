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

#ifndef NET_H
#define NET_H

#include <types.h>
#include "net_def.h"
#include <os.h>

#define NET_IPV4_ADDR    "192.168.1.100"

//#define DEBUGNET

struct net_t {
    uint32 arpto;

    struct os_queue_t *qtx;
    struct os_queue_t *qproc;

#define NET_EVENT_MASK_INIT    (1 << 0)
#define NET_EVENT_MASK_NET_IRQ (1 << 1)
    BASE_TYPE events;
#define NET_MUTEX_MASK_ARP     (1 << 0)
#define NET_MUTEX_MASK_UDP     (1 << 1)
    BASE_TYPE mutex;

    uint16 udp_ident;
};

extern struct net_t net;

void net_task();
void net_rx_task();
void net_process_task();

void net_receive(struct net_eth_frame_t *frame, int len);
void net_send(struct net_eth_frame_t *frame, int len);

typedef void (*pudp_cb)(uint32, uint32, uint16, uint16, uint8 *, uint16);

void net_udp_receive(uint32 src_ip, uint32 dst_ip, uint16 src_port, uint16 dst_port, uint8 *payload, uint16 length);
void net_udp_send(uint32 src_ip, uint32 dst_ip, uint16 src_port, uint16 dst_port, uint8 *payload, uint16 length);
void net_udp_add_cb(uint16 port, pudp_cb cb);
void net_udp_delete_cb(pudp_cb cb);

uint32 net_aton(char *ip);

#endif

