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

#ifndef DPORT_PROTO_H
#define DPORT_PROTO_H

#include <types.h>

/*
 * Packet format
 * =============
 *
 *     % <cmd> <payload> <CS> CR LF
 *
 *         %          start of packet
 *         cmd        one symbol [a-zA-Z]
 *         payload    hex digits [0-9A-Z]
 *         CS         sum of "cmd" (ASCII value) and payload in binary form
 *                    truncated to one byte
 *
 *  Examples
 *  --------
 *
 *     XXX invalid check in this example (XOR'ed instead of addition)
 *     %m84C021A004000000AC    read 4 bytes of memory at 0xa021c084
 *
 *
 */

#define DPORT_CMD_MARK    '%'

#define DPORT_CMD_REQ_BUFSIZE         'a'
#define DPORT_CMD_REQ_BREAK           'b'
#define DPORT_CMD_REQ_CONTINUE        'c'
#define DPORT_CMD_REQ_FLASH           'f'
#define DPORT_CMD_REQ_READREG         'g'
#define DPORT_CMD_REQ_WRITEREG        'G'
#define DPORT_CMD_REQ_FLASH_BUFSIZE   'l'
#define DPORT_CMD_REQ_READMEM         'm'
#define DPORT_CMD_REQ_WRITEMEM        'M'
#define DPORT_CMD_REQ_RESET           'r'
#define DPORT_CMD_REQ_USERF           'u'
#define DPORT_CMD_REQ_STEP            's'

#define DPORT_CMD_RESP_CSERROR    'E'
#define DPORT_CMD_RESP_CMDUNKNOWN 'U'
#define DPORT_CMD_RESP_ERRCUSTOM  'e'

#define DPORT_MAXBUF_SIZE          4096

#define DPORT_HEAD_SIZE            2
#define DPORT_TAIL_SIZE            4
#define DPORT_HEADTAIL_SIZE        (DPORT_HEAD_SIZE + DPORT_TAIL_SIZE)
#define DPORT_PAYLOAD_SIZE         (DPORT_MAXBUF_SIZE - DPORT_HEADTAIL_SIZE)
#define DPORT_BINARY_PAYLOAD_SIZE  (DPORT_PAYLOAD_SIZE / 2)

#define DPORT_BINARY_PAYLOAD_SIZE2(buflen)  ((buflen - DPORT_HEADTAIL_SIZE) / 2)

#pragma pack(push, 1)
union dport_req_t {
    struct dport_req_read_reg_t {
        uint8 regn;
    } read_reg;
    struct dport_req_write_reg_t {
        uint8 regn;
        uint32 value;
    } write_reg;
    struct dport_req_read_mem_t {
        uint32 offset;
        uint32 length;
    } read_mem;
    struct dport_req_write_mem_t{
        uint32 offset;
        uint32 length;
#define WRITEMEM_PAYLOAD_MAX_SIZE    (DPORT_BINARY_PAYLOAD_SIZE - 8)
        union {
            uint8 data[WRITEMEM_PAYLOAD_MAX_SIZE];
            uint8  reg8;
            uint16 reg16;
            uint32 reg32;
        };
    } write_mem;
    struct {
#define DPORT_CMD_FLASH_ARG_SIZE    8
        uint32 offset;
        uint32 length;
#define FLASH_PAYLOAD_MAX_SIZE    (DPORT_BINARY_PAYLOAD_SIZE - DPORT_CMD_FLASH_ARG_SIZE)
        uint8 data[FLASH_PAYLOAD_MAX_SIZE];
    } flash;
    struct dport_req_userf_t {
        uint8 n;
    } userf;
};

union dport_resp_t {
    struct {
        uint8 regn;
        uint32 value;
    } read_reg;
    struct dport_resp_write_reg_t {
        uint8 regn;
    } write_reg;
    struct dport_resp_flash_bufsize_t {
        uint32 size;
    } flash_bufsize;
    struct {
        uint32 addr;
    } brk;
    struct {
        uint16 length;
    } bufsize;
    struct {
        uint32 offset;
        uint32 length;
#define READMEM_PAYLOAD_MAX_SIZE    (DPORT_BINARY_PAYLOAD_SIZE - 8)
        union {
            uint8 data[READMEM_PAYLOAD_MAX_SIZE];
            uint8  reg8;
            uint16 reg16;
            uint32 reg32;
        };
    } read_mem;
    struct dport_write_mem_resp_t {
        uint32 offset;
        uint32 length;
    } write_mem;
    struct {
        uint32 offset;
        uint32 length;
    } flash;
};
#pragma pack(pop)

#define DPORT_REGNUM_R0              0
#define DPORT_REGNUM_R1              1
#define DPORT_REGNUM_R2              2
#define DPORT_REGNUM_R3              3
#define DPORT_REGNUM_R4              4
#define DPORT_REGNUM_R5              5
#define DPORT_REGNUM_R6              6
#define DPORT_REGNUM_R7              7
#define DPORT_REGNUM_R8              8
#define DPORT_REGNUM_R9              9
#define DPORT_REGNUM_R10            10
#define DPORT_REGNUM_R11            11
#define DPORT_REGNUM_R12            12
#define DPORT_REGNUM_SP             13
#define DPORT_REGNUM_LR             14
#define DPORT_REGNUM_PC             15
#define DPORT_REGNUM_XPSR           25

#endif

