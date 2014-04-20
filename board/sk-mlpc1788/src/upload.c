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

/*
 * upload binary data to device via some of transport
 */
#include <string.h>
#include <debug.h>
#include "upload.h"
#include "net/net.h"
#include "crc.h"



#define UPLOAD_TEMP_SIZE    (2 * 1024 * 1024)
uint8 upload_temp[UPLOAD_TEMP_SIZE];

struct upload_state_t {
    uint16 type;
    uint16 crc;
    uint32 len;
    uint32 maxlen;
};

static struct upload_state_t ustate;

static int upload_process(struct upload_req_t *req, int ilen, struct upload_resp_t *resp);

static void upload_start(uint16 type);
static void upload_data(uint16 type, uint8 *data, int len, uint32 offset);
static uint16 upload_end(uint16 type);

/*
 *
 */
void upload_usb_receive(struct usbdev_ep_t *ep, uint8 *payload, int len)
{
    int wr;
#define BUF_SIZE   64
    uint8 buf[BUF_SIZE];

    dprint("slb", "upload: ", len, payload);

    wr = upload_process((struct upload_req_t *)payload, len, (struct upload_resp_t*)buf);
    if (wr > 0)
        usbdev_send(ep, buf, wr, OS_WAIT_FOREVER);
}

/*
 *
 */
static int upload_mkerror(struct upload_resp_t *resp, uint8 error)
{
    resp->head.cmd        = UPLOAD_CMD_ERROR;
    resp->cmd_error.error = error;
    return (sizeof(struct upload_cmd_head_t) + sizeof(struct upload_cmd_end_error_t));
}

/*
 *
 */
static int upload_process(struct upload_req_t *req, int ilen, struct upload_resp_t *resp)
{
    int olen;

    olen = 0;
    resp->head.type = req->head.type; /* NOTE */
    resp->head.cmd  = req->head.cmd;
    switch (req->head.cmd)
    {
        case UPLOAD_CMD_START:
            if (ilen != (sizeof(struct upload_cmd_start_req_t) + sizeof(struct upload_cmd_head_t)))
                return upload_mkerror(resp, UPLOAD_ERROR_CMD_LENMISMATCH);

            switch (ustate.type)
            {
                case UPLOAD_TYPE_TEMP:
                    resp->cmd_start.maxlen = UPLOAD_TEMP_SIZE;
                    break;
                default:
                    return upload_mkerror(resp, UPLOAD_ERROR_TYPE_UNKNOWN);
            }
            ustate.type   = req->head.type;
            ustate.crc    = req->cmd_start.crc;
            ustate.len    = req->cmd_start.len;
            ustate.maxlen = resp->cmd_start.maxlen;

            upload_start(ustate.type);

            olen = sizeof(struct upload_cmd_start_resp_t);
            break;
        case UPLOAD_CMD_PAYLOAD:
            {
                int len;

                len = req->cmd_payload.len;
                if (ilen != (len + 6 + sizeof(struct upload_cmd_head_t)))
                    return upload_mkerror(resp, UPLOAD_ERROR_CMD_LENMISMATCH);

                if ((req->cmd_payload.offset + len) > ustate.maxlen)
                    return upload_mkerror(resp, UPLOAD_ERROR_TYPE_OFFSET);
                upload_data(req->head.type, req->cmd_payload.payload, len, req->cmd_payload.offset);
            }
            break;
        case UPLOAD_CMD_END:
            if (ilen != sizeof(struct upload_cmd_head_t))
                return upload_mkerror(resp, UPLOAD_ERROR_CMD_LENMISMATCH);
            resp->cmd_end.crc = upload_end(ustate.type);
            olen = sizeof(struct upload_cmd_end_resp_t);
        default:
            return upload_mkerror(resp, UPLOAD_ERROR_CMD_UNKNOWN);
    }
    resp->head.type = ustate.type; /* NOTE */

    olen += sizeof(struct upload_cmd_head_t);

    return olen;
}

/*
 * start of uploadх
 */
static void upload_start(uint16 type)
{
    dprint("sn", "upload start");
    dprint("s_2xn", "    CRC    =",  ustate.crc);
    dprint("s_4xn", "    LENGTH =",  ustate.len);

}

/*
 * actually load data of specific type
 */
static void upload_data(uint16 type, uint8 *data, int len, uint32 offset)
{
    dprint("s_4x_s4xn", "upload_data, offset", offset, "length", len);

    /* XXX upload data to temporary buffer */
    memcpy(&upload_temp[offset], data, len);
}

/*
 * end of upload
 *
 * RETURN   chechsum
 */
static uint16 upload_end(uint16 type)
{
    uint16 crc;

    dprint("sn", "upload end");

    /* XXX calculate crc on temporary buffer */
    crc16ccitt_init(&crc);
    crc16ccitt_update(&crc, upload_temp, ustate.len);

    if (crc == ustate.crc)
    {
        dprint("sn", "crc match");
    }

    return crc;
}

