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

#include "gs.h"
#include "gs_prim.h"

/*
 *
 */
void gs_prim_draw_rect(struct gs_win_t *win,
        int x0, int y0, int x1, int y1, int lwidth, uint32 color)
{
    int cnt;
    union gs_pixel_t *mem;

    if (!win)
        return;

    if (lwidth == 0 ||
        x0 > x1 ||
        y0 > y1 || 
        x1 >= win->width ||
        y1 >= win->height)
        return;

    /* upper and bottom lines */
    mem = &win->mem[win->width * y0 + x0];
    cnt = x1 - x0;
    while (cnt--)
    {
        mem->value = color;
        mem++;
    }
    mem = &win->mem[win->width * y1 + x0];
    cnt = x1 - x0 + 1;
    while (cnt--)
    {
        mem->value = color;
        mem++;
    }

    /* left and right lines */
    mem = &win->mem[win->width * y0 + x0];
    cnt = y1 - y0;
    while (cnt--)
    {
        mem->value = color;
        mem += win->width;
    }
    mem = &win->mem[win->width * y0 + x1];
    cnt = y1 - y0 + 1;
    while (cnt--)
    {
        mem->value = color;
        mem += win->width;
    }

    /* XXX recurse */
    if (lwidth > 1)
    {
        x0++; y0++;
        x1--; y1--;
        gs_prim_draw_rect(win, x0, y0, x1, y1, lwidth - 1, color);
    }
}

/*
 *
 */
void gs_prim_fill_rect(struct gs_win_t *win,
        int x0, int y0, int x1, int y1, uint32 color)
{
    int x;
    union gs_pixel_t *mem;

    if (!win)
        return;

    if (x0 > x1 ||
        y0 > y1 || 
        x1 >= win->width ||
        y1 >= win->height)
        return;

    for (; y0 <= y1; y0++)
    {
        mem = &win->mem[win->width * y0 + x0];
        for (x = x0; x <= x1; x++)
        {
            mem->value = color;
            mem++;
        }
    }
}

