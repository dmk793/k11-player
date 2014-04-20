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

#ifndef UPLOAD_SHARED_H
#define UPLOAD_SHARED_H

#define UPLOAD_MAX_PAYLOAD_SIZE    32

#include <types.h>

/* XXX check net.h on target */
#define UPLOAD_TARGET_IP    "192.168.1.100"

#define UPLOAD_HOST_PORT    9930
#define UPLOAD_TARGET_PORT  9930

/***************************************
 * Upload types
 ***************************************/
#define UPLOAD_TYPE_TEMP       0
#define UPLOAD_TYPE_FONT8X8    1

/***************************************
 * Protocol
 ***************************************/

#define UPLOAD_CMD_START    0x00
#define UPLOAD_CMD_PAYLOAD  0x01
#define UPLOAD_CMD_END      0x02
#define UPLOAD_CMD_ERROR    0xFF

#define UPLOAD_ERROR_CMD_LENMISMATCH  0xF0
#define UPLOAD_ERROR_CMD_UNKNOWN      0xF1
#define UPLOAD_ERROR_CMD_TYPEMISMATCH 0xF2
#define UPLOAD_ERROR_TYPE_UNKNOWN     0xF3
#define UPLOAD_ERROR_TYPE_OFFSET      0xF4

#pragma pack(push, 1)
/* ********* */
struct upload_cmd_head_t {
    uint8 cmd;
    uint16 type; /* type of payload */
};
/* ********* */
struct upload_cmd_start_req_t {
    uint16 crc;  /* crc16 ccitt of whole upload unit */
    uint32 len;  /* length of whole upload unit */
};
struct upload_cmd_start_resp_t {
    uint32 maxlen;  /* maximum length accepted by target */
};
/* ********* */
struct upload_cmd_payload_req_t {
    uint32 offset;     /* offset of data */
    uint16 len;        /* payload length
                          NOTE can be extracted from transport,
                          but will it be for ease of processing
                        */
    uint8 payload[UPLOAD_MAX_PAYLOAD_SIZE];
};
struct upload_cmd_payload_resp_t {
    uint32 offset;
};
/* ********* */
struct upload_cmd_end_resp_t {
    uint16 crc;  /* crc16 ccitt of whole upload unit */
};
/* ********* */
struct upload_cmd_end_error_t {
    uint8 error;
};
/* ********* */

struct upload_req_t {
    struct upload_cmd_head_t head;
    union {
        struct upload_cmd_start_req_t   cmd_start;
        struct upload_cmd_payload_req_t cmd_payload;
    };
};

struct upload_resp_t {
    struct upload_cmd_head_t head;
    union {
        struct upload_cmd_start_resp_t   cmd_start;
        struct upload_cmd_payload_resp_t cmd_payload;
        struct upload_cmd_end_resp_t     cmd_end;
        struct upload_cmd_end_error_t    cmd_error;
    };
};
#pragma pack(pop)

#endif

