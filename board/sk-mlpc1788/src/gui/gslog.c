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

#include <os.h>
#include <debug.h>
#include <string.h>
#include "gslog.h"
#include "../gs/gs.h"
#include "../slog.h"

#define ADD_CB() {               \
    os_disable_irq();            \
    debug_add_cb(slog_append);   \
    os_enable_irq();             \
}

#define DELETE_CB() {              \
    os_disable_irq();              \
    debug_delete_cb(slog_append);  \
    os_enable_irq();               \
}

#define GSLOG_WIN_POSX    (0)
#define GSLOG_WIN_POSY    (0)
#define GSLOG_WIN_WIDTH   (GS_RES_HORIZONTAL - 32)
#define GSLOG_WIN_HEIGHT  (GS_RES_VERTICAL   - 64)
#define GSLOG_FONT        GS_FONT_7X8
//#define GSLOG_FONT        GS_FONT_9X16
#define GSLOG_FONT_COLOR  GS_COLOR_YELLOW

#define GSLOG_PADX        4
#define GSLOG_PADY        4

/*
 *
 */
void gslog_task()
{
    int i, y, loglines, linelength, winlines, lwin0, lwin1;
#define LINE_SIZE_MAX    128
    char line[LINE_SIZE_MAX];
    struct gs_win_t *win;

    while (!gs_initialized())
        os_wait_ms(10);

    win = gs_win_create(GSLOG_WIN_POSX, GSLOG_WIN_POSY, GSLOG_WIN_WIDTH, GSLOG_WIN_HEIGHT);
    if (!win)
    {
        dprint("sn", ERR_PREFIX "Failed to create log window");
        os_wait(OS_WAIT_FOREVER);
    }
    gs_win_set_border(win, 1, GS_COLOR_WHITE);
    gs_win_refresh(win);

    winlines   = (GSLOG_WIN_HEIGHT - (GSLOG_PADY * 2)) / GSLOG_FONT->height;
    linelength = (GSLOG_WIN_WIDTH  - (GSLOG_PADX * 2)) / GSLOG_FONT->width;

    while (1)
    {
        os_event_wait(&slog_event, SLOG_EVENT_MASK_UPDATE, OS_FLAG_CLEAR, OS_WAIT_FOREVER);

        loglines = slog_lines();
        lwin0 = 0;
        if (loglines > winlines)
            lwin0 = loglines - winlines;
        lwin1 = loglines - 1;

        for (y = 0, i = lwin0; i <= lwin1; i++)
        {
            memset(line, ' ', linelength);
            slog_get_line(line, LINE_SIZE_MAX, i);
            line[strlen(line)] = ' ';
            line[linelength] = 0;

            gs_text_put(win, line, GSLOG_PADX, GSLOG_PADY + y, GSLOG_FONT_COLOR, win->bgcolor.value, GSLOG_FONT);

            y += GSLOG_FONT->height;
        }
        gs_win_refresh(win);
    }
}

//        DELETE_CB();
//
//        n = slog_lines();
//        dprint("s 4ds", "lines = ", n, "\r\n");
//        for (i = 0; i < n; i++)
//        {
//            dprint("sss", "line:", slog_get_line(line, LINE_SIZE_MAX, i), ":line\r\n");
//        }
//
//        ADD_CB();

