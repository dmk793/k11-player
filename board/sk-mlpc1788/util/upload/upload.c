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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <upload_shared.h>
#include <types.h>
#include "commdev.h"
#include "crc16.h"

static void upload_file(uint16 type, int fd, struct commdev_t *commdev);
static void upload_check_response(struct upload_req_t *req, struct upload_resp_t *resp, int resplen);
static uint32 upload_start(struct commdev_t *commdev, uint16 type, uint16 crc, uint32 len);

/*
 *
 */
int main(int argc, char **argv)
{
    char *type_arg;
    char *file_arg;
    int fdesc;
    uint16 type;
    struct commdev_t commdev;

    if (argc < 3)
    {
        printf("Upload binary data to device\r\n");
        printf("Usage: \n");
        printf("    %s [type] [file]\n", argv[0]);
        printf("type:\n");
        printf("    TEMP       temporary storage\n");
        printf("    FONT8X8    8x8 raster font\n");
        printf("\n");
        return 0;
    }

    type_arg = argv[1];
    file_arg = argv[2];

    if (strcmp(type_arg, "FONT8X8") == 0)
    {
        type = UPLOAD_TYPE_FONT8X8;
    } else if (strcmp(type_arg, "TEMP") == 0) {
        type = UPLOAD_TYPE_TEMP;
    } else {
        fprintf(stderr, "(E) Unknown type \"%s\"\n", type_arg);
        return 1;
    }

    fdesc = open(file_arg, O_RDONLY);
    if (fdesc < 0)
    {
        fprintf(stderr, "(E) Can not open file \"%s\": %s\n", file_arg, strerror(errno));
        return 1;
    }

    commdev_init();
    commdev_open(&commdev, 0);

    upload_file(type, fdesc, &commdev);

    commdev_close(&commdev);
    commdev_exit();
    close(fdesc);
    return 0;
}


#define TIMEOUT    500

/*
 *
 */
static void upload_file(uint16 type, int fd, struct commdev_t *commdev)
{
    int rd;
    uint32 len;
    uint8 buf[UPLOAD_MAX_PAYLOAD_SIZE];
    uint16 crc;
    uint32 maxlen;

    len = 0;
    crc16ccitt_init(&crc);
    /* calculate CRC of whole file */
    do {
        rd = read(fd, buf, UPLOAD_MAX_PAYLOAD_SIZE);
        if (rd < 0)
        {
            fprintf(stderr, "(E) File read error: %s\n", strerror(errno));
            exit(errno);
        }
        len += rd;
        crc16ccitt_update(&crc, buf, rd);
    } while(rd > 0);

    printf("CRC    = %04hX\n", crc);
    printf("LENGTH = %08X\n", len);

    maxlen = upload_start(commdev, type, crc, len);

    if (maxlen < len)
    {
        fprintf(stderr, "(E) File length exceed maximum length supported by device: %u/%u\n", maxlen, len);
        return;
    }

}

/*
 *
 */
static void upload_check_response(struct upload_req_t *req, struct upload_resp_t *resp, int resplen)
{
    if (resp->head.cmd == UPLOAD_CMD_ERROR)
    {
        fprintf(stderr, "(E) Device returned error: %s (%u)\n", "test", resp->cmd_error.error);
        exit(1);
    }
}

/*
 *
 */
static uint32 upload_start(struct commdev_t *commdev, uint16 type, uint16 crc, uint32 len)
{
    struct upload_req_t req;
    struct upload_resp_t resp;
    uint32 rd;

    req.head.cmd  = UPLOAD_CMD_START;
    req.head.type = type;
    req.cmd_start.crc = crc;
    req.cmd_start.len = len;

//    /* XXX receive all unhandled data */
//    commdev_recv(commdev, (uint8 *)&resp, sizeof(struct upload_resp_t), TIMEOUT);

    commdev_send(commdev, (uint8*)&req,
            sizeof(struct upload_cmd_head_t) + sizeof(struct upload_cmd_start_req_t), TIMEOUT);

    rd = commdev_recv(commdev, (uint8 *)&resp, sizeof(struct upload_resp_t), TIMEOUT);
    printf("rd = %d\n", rd);

    upload_check_response(&req, &resp, rd);

    return resp.cmd_start.maxlen;
}

