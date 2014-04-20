#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "errcode.h"
#include "comm.h"
#include "dport_proto.h"

//#define DEBUG_COMM

#define COMM_RETRY_COUNT    4
#define COMM_SEND_TIMEOUT   100
#define COMM_RECV_TIMEOUT   500

static uint16 comm_num2dup(uint8 num);
static uint8 comm_dup2num(uint16 dup);
static uint8 comm_cs(uint8 *cs, uint8 *data, int len);

static int comm_recv(struct commdev_t *commdev, struct debug_t *debug);
static int comm_getpacket(struct commdev_t *commdev, struct debug_t *debug);
static int comm_checkpacket(struct debug_t *debug);

/*
 *
 */
void comm_mkreq(struct debug_buffer_t *buf, uint8 cmd, uint8 *payload, int len)
{
    uint16 *data;
    uint8 cs;
    int dlen;

    /* TODO length sanity check */
    dlen = len * 2;
    if (dlen > (DEBUG_BUF_SIZE - DPORT_HEADTAIL_SIZE))
    {
        fprintf(stderr, "(E) overbuffer, %s", __FUNCTION__);
        exit(ERRCODE_OVERBUFFER);
    }

    buf->buf[0] = DPORT_CMD_MARK;
    buf->buf[1] = cmd;
    data = (uint16*)&buf->buf[2];

    cs = comm_cs(&buf->buf[1], NULL, 0);
    if (payload)
    {
        while (len--)
        {
            cs = comm_cs(&cs, payload, 1);
            *data++ = comm_num2dup(*payload++);
        }
    }
    *data++ = comm_num2dup(cs);

    *data = (('\n' << 8) | '\r') & 0xffff;

    buf->c = dlen + DPORT_HEADTAIL_SIZE;
}

/*
 * ARGS
 *     commdev    communication device structure
 *     debug      debug structure
 *
 * RETURN
 *     0 on success, -1 on error
 *
 */
int comm_txrx(struct debug_t *debug)
{
    struct commdev_t *commdev;
    int ntry;

    commdev = debug->commdev;

    /* XXX not all operation can be simply retried (memory write, flash write, etc) */
    ntry = COMM_RETRY_COUNT;
    while (ntry--)
    {
#ifdef DEBUG_COMM
    {
        int i;

        printf(">> ");
        for (i = 0; i < debug->tx.c; i++)
            printf("%c", debug->tx.buf[i]);
    }
#endif

        if (commdev_send(commdev,
                    debug->tx.buf, debug->tx.c, COMM_SEND_TIMEOUT) < 0)
        {
            fprintf(stderr, "(E) txrx, send error/timeout\n");
            goto retry;
        }

        if (comm_recv(commdev, debug) < 0)
        {
            fprintf(stderr, "(E) txrx, recv error\n");
            goto retry;
        }

        /* XXX check command */
        if (debug->tx.buf[1] != debug->rx.buf[1])
        {
            fprintf(stderr, "(E) recv command mismatch , tx \"%c\", rx \"%c\", %s\n",
                    debug->tx.buf[1], debug->rx.buf[1], __FUNCTION__);
            goto retry;
        }
        return 0;
retry:
        if (ntry)
            fprintf(stderr, "(W) %u try remains for '%c' command\n", ntry, debug->tx.buf[1]);
    }
    fprintf(stderr, "(E) txrx error\n");
    return -1;
}

/*
 * receive, decode packet
 *
 * RETURN
 *     0 on success, -1 on error
 */
static int comm_recv(struct commdev_t *commdev, struct debug_t *debug)
{
    if (comm_getpacket(commdev, debug) < 0)
        return -1;

    return comm_checkpacket(debug);
}

/*
 * get packet (without check)
 *
 * RETURN
 *     0 on success, -1 on error
 */
static int comm_getpacket(struct commdev_t *commdev, struct debug_t *debug)
{
    int rd, maxlen, rxc, n;
    uint8 *rxbuf, *cbuf;

    rxbuf  = debug->rx.buf;
    maxlen = DEBUG_BUF_SIZE;
    rxc    = 0;

    while (1)
    {
        if (maxlen == 0)
        {
            fprintf(stderr, "(E) overbuffer, %s\n", __FUNCTION__);
            return -1;
        }

        rd = commdev_recv(commdev, rxbuf, maxlen, COMM_RECV_TIMEOUT);
        if (rd <= 0)
        {
            fprintf(stderr, "(E) recv error/timeout, %s\n", __FUNCTION__);
            return -1;
        }
        rxc    += rd;
        maxlen -= rd;

        /* check for mark */
        cbuf = debug->rx.buf;
        rxbuf += rd;
        while (*cbuf != DPORT_CMD_MARK && rxc)
        {
            rxc--;
            maxlen++;
            cbuf++;
            rxbuf--;
        }
        if (cbuf != debug->rx.buf && rxc)
            memmove(debug->rx.buf, cbuf, rxc);

        if (!rxc)
            continue;

        /* check for \n */
        cbuf = &debug->rx.buf[1];
        n = 1;
        while (*cbuf != '\n' && n < rxc)
        {
            cbuf++;
            n++;
        }
        if (n >= rxc)
            continue;

        if (*(cbuf - 1) != '\r')
        {
            fprintf(stderr, "(E) CR missed, %s\n", __FUNCTION__);
            return -1;
        }

        n++; /* NOTE */

#ifdef DEBUG_COMM
        {
            int k;

            printf("<  ");
            for (k = 0; k < n; k++)
                printf("%c", debug->rx.buf[k]);
        }
#endif

        debug->rx.c = n;
        return 0;
    }

    return -1;
}

/*
 * check packet
 *
 * RETURN
 *     0 on success, -1 on error
 */
static int comm_checkpacket(struct debug_t *debug)
{
    int rxc, dlen;
    uint8 *rxbuf;
    uint8 cs;

    rxc   = debug->rx.c;
    rxbuf = debug->rx.buf;
    
    /*
     * Command should have even number of symbols.
     * And at least 6 symbols - one symbol for MARK,
     * one symbol for command, two symbols for checksum; CR symbol,
     * LF symbol.
     */
    if ((rxc % 2) != 0)
    {
        fprintf(stderr, "(E) recv odd number of symbols, %s\n", __FUNCTION__);
        return -1;
    }

#ifndef DEBUG_COMM
    /* TODO XXX transfrom error to text? */
    if (rxbuf[1] == DPORT_CMD_RESP_ERRCUSTOM)
    {
        int i;

        printf("<  ");
        for (i = 0; i < rxc; i++)
            printf("%c", rxbuf[i]);
    }
#endif

    /* transform payload of command to binary  */
    {
        uint8 *in;
        uint8 *out;

        in  = &rxbuf[2];
        out = &rxbuf[2];
        /* length of hexadecimal data transformed to binary */
        dlen = (rxc - DPORT_HEADTAIL_SIZE) / 2;
        while (*in != '\r')
        {
            *out = comm_dup2num(*((uint16*)in));
            in   += 2;
            out  += 1;
        }
    }

    /* check CS */
    cs = comm_cs(&rxbuf[1], NULL, 0);
    cs = comm_cs(&cs, &rxbuf[2], dlen);
    if (cs != rxbuf[2 + dlen])
    {
        fprintf(stderr, "(E) recv checksum mismatch, %s\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

/*
 *
 */
static uint16 comm_num2dup(uint8 num)
{
    uint16 ret;
    uint8  n;

    ret = 0;
    n   = num & 0x0f;
    if (n < 10)
        ret |= (n + '0');
    else
        ret |= (n + 'A' - 10);

    n   = num >> 4;
    ret <<= 8;
    if (n < 10)
        ret |= (n + '0');
    else
        ret |= (n + 'A' - 10);

    return ret;
}

/*
 *
 */
static uint8 comm_dup2num(uint16 dup)
{
    uint8 ret;
    uint8 n;

    ret = 0;
    n = dup & 0xff;
    if (n < 'A')
        ret |= (n - '0'     ) & 0x0f;
    else
        ret |= (n - 'A' + 10) & 0x0f;

    ret <<= 4;
    n = dup >> 8;
    if (n < 'A')
        ret |= (n - '0'     ) & 0x0f;
    else
        ret |= (n - 'A' + 10) & 0x0f;

    return ret;
}

/*
 *
 */
static uint8 comm_cs(uint8 *cs, uint8 *data, int len)
{
    uint8 ret;

    ret = 0;
    if (cs)
        ret = *cs;
    if (data)
    {
        while (len--)
            ret += *data++;
    }

    return ret & 0xff;
}

