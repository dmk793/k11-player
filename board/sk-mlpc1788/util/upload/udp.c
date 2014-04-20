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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <upload_shared.h>
#include "transport.h"

static int sock;
static int fdesc;
struct sockaddr_in si_target;

static int open_socket();
static int send_data(uint8 *payload, int length);
//static int recv_data(uint8 *payload, int length);


/*
 *
 */
static int open_socket()
{
    int s;
    struct sockaddr_in si_host;

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        fprintf(stderr, "(E) Can not create socket: %s\n", strerror(errno));
        exit(1);
    }

    memset((char *) &si_host, 0, sizeof(si_host));
    si_host.sin_family      = AF_INET;
    si_host.sin_port        = htons(UPLOAD_HOST_PORT);
    si_host.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (const struct sockaddr *)&si_host, sizeof(si_host)) < 0)
    {
        fprintf(stderr, "(E) Can not bind name to socket: %s\n", strerror(errno));
        exit(1);
    }

    /* initialize also target address */
    memset((char *) &si_target, 0, sizeof(si_target));
    si_target.sin_family = AF_INET;
    si_target.sin_port = htons(UPLOAD_TARGET_PORT);
    if (inet_aton(UPLOAD_TARGET_IP, &si_target.sin_addr) == 0) {
        fprintf(stderr, "(E) inet_aton() failed\n");
        exit(1);
    }

    return s;
}

/*
 *
 */
static int send_data(uint8 *payload, int length)
{
    int wr;

    wr = sendto(sock, payload, length, 0,
            (const struct sockaddr *)&si_target, sizeof(si_target));
    if (wr < 0)
    {
        fprintf(stderr, "(E) sendto() failed, %s\n", strerror(errno));
        exit(1);
    }

    return wr;
}

/*
 *
 * RETURN
 *    0 on timeout, length of response otherwise
 */
static int recv_data(uint8 *payload, int maxsize)
{
    int rd;
    int sel;
    socklen_t slen;
    fd_set rfs;
    struct timeval to;

    rd   = 0;
    slen = sizeof(si_target);

    FD_ZERO(&rfs);
    FD_SET(sock, &rfs);

    to.tv_sec  = RECV_TIMEOUT / 1000;
    to.tv_usec = (RECV_TIMEOUT % 1000) * 1000;

    sel = select(sock + 1, &rfs, NULL, NULL, &to);
    if (sel < 0)
    {
        fprintf(stderr, "(E) select() failed, %s\n", strerror(errno));
        exit(1);
    }

    if (sel == 0)
        return 0;

    if (sel > 0 && FD_ISSET(sock, &rfs))
    {
        rd = recvfrom(sock, payload, maxsize, 0, (struct sockaddr *)&si_target,
                &slen);
        if (rd < 0)
        {
            fprintf(stderr, "(E) recvfrom() failed, %s\n", strerror(errno));
            exit(1);
        }
    }

    return rd;
}
