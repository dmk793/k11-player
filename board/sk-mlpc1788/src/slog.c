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
 *
 */
#include <string.h>
#include "slog.h"

#define SLOG_BUF_SIZE         2048
#define SLOG_MAX_LINE_LENGTH  64

struct slog_t {
    uint8 buf[SLOG_BUF_SIZE];
    int cnt;
    int wp;

    int linelen;
};

struct slog_t slog;
#define MUTEX_MASK    (1 << 0)
static BASE_TYPE mutex;
BASE_TYPE slog_event;

static void slog_addchar(int ch);

/*
 * append character to system log
 *
 * NOTE
 *     lock with mutex is big waste of cpu time
 *
 */
int slog_append(int ch)
{
    /* suppress nonprint characters except LF */
    if (ch != '\n' && ch < 0x20)
        return 0;

    slog_lock();
    {
        slog_addchar(ch);

        if (ch != '\n')
        {
            slog.linelen++;
            if (slog.linelen >= SLOG_MAX_LINE_LENGTH)
            {
                slog_addchar('\\');
                slog_addchar('\n');
                slog.linelen = 0;
            }
        } else {
            slog.linelen = 0;
        }
    }
    slog_unlock();
    /* raise update event */
    if (ch == '\n')
        os_event_raise_ns(&slog_event, SLOG_EVENT_MASK_UPDATE);

    return ch;
}

/*
 *
 */
static void slog_addchar(int ch)
{
    /* sanity check, NOTREACHED */
    if (slog.wp >= SLOG_BUF_SIZE)
        return;

    slog.buf[slog.wp] = ch;
    slog.wp++;
    if (slog.wp >= SLOG_BUF_SIZE)
        slog.wp = 0;
    if (slog.cnt < SLOG_BUF_SIZE)
        slog.cnt++;
}

/*
 * lock system log from modifications
 */
void slog_lock()
{
    os_mutex_lock(&mutex, MUTEX_MASK, OS_FLAG_NONE, OS_WAIT_FOREVER);
}

/*
 * unlock system log
 */
void slog_unlock()
{
    os_mutex_unlock_ns(&mutex, MUTEX_MASK);
}

/*
 * get number of lines contained in system log
 *
 * RETURN
 *      number of lines
 */
int slog_lines()
{
    int lines;
    int rp;
    int cnt;

    lines = 0;
    slog_lock();
    {
        cnt = slog.cnt;
        if (slog.cnt <= slog.wp)
            rp = slog.wp - slog.cnt;
        else
            rp = SLOG_BUF_SIZE - (slog.cnt - slog.wp);

        while (cnt--)
        {
            if (slog.buf[rp] == '\n')
                lines++;
            rp++;
            if (rp >= SLOG_BUF_SIZE)
                rp = 0;
        }
    }
    slog_unlock();

    return lines;
}

/*
 * get corresponding line from system log
 *
 * NOTE result line will be NULL terminated, LF character not included
 *
 * ARGS
 *     st       pointer where line from system log will be stored
 *     maxsize  maximum length of line
 *     n        line index from begining of system log
 *
 * RETURN
 *     NULL if line not found, pointer to line otherwise (st argument itself)
 *     
 */
char* slog_get_line(char *st, int maxsize, int n)
{
    char *ret, *stc;
    int line;
    int rp;
    int cnt;

    if (!st || maxsize < 1)
        return NULL;

    ret = NULL;
    stc = st;
    slog_lock();
    {
        line = 0;
        cnt = slog.cnt;
        if (slog.cnt <= slog.wp)
            rp = slog.wp - slog.cnt;
        else
            rp = SLOG_BUF_SIZE - (slog.cnt - slog.wp);

        /* find corresponding line  */
        if (n > 0)
        {
            while (cnt--)
            {
                if (slog.buf[rp++] == '\n')
                {
                    line++;
                    if (line == n)
                        break;
                }
                if (rp >= SLOG_BUF_SIZE)
                    rp = 0;
            }
        }
        if (line != n)
            goto out;

        /* catch data */
        while (cnt--)
        {
            if (slog.buf[rp] == '\n')
                break;

            if (--maxsize)
                *st++ = slog.buf[rp];
            else
                break;

            rp++;
            if (rp >= SLOG_BUF_SIZE)
                rp = 0;
        }
        *st = 0;
    }
    ret = stc;
out:
    slog_unlock();
    return ret;
}

