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

#include <debug.h>
#include <string.h>
#include "gs.h"
#include "gsp.h"
#include "gs_text.h"

/*
 *
 *
 *
 */
void _gs_text_put(struct gs_win_t *win,
        char *str, int slen, int x, int y, int color, int bgcolor, struct gs_font_t *font)
{
    int mask, v, h;
    uint8 *data;
    int len;
    union gs_pixel_t *mem;
    int idx;

    if (!win)
        return;

    if (str == NULL || *str == 0 || font == NULL || font->data == NULL)
        return;

    /* sanity check of y coordinate */
    if (slen)
        len = slen;
    else
        len = strlen(str);
    if (y + font->height > win->height)
        return;
    /* output part of line if it is too large to fit on window */
    if (x + (len * font->width) > win->width)
        len = (win->width - x) / font->width;

    /* loop thru string */
    for (;*str && len;str++, len--)
    {
        idx = *str;
        if (idx > font->offset)
            idx -= font->offset;
        else
            idx = 0;

        if (idx >= font->nsym)
            idx = 0; /* NOTE print first character of font */

        data = &font->data[font->width * font->height * idx / 8];

        /* draw horizontal data */
        for (h = 0; h < font->width; h++)
        {
            mem = &win->mem[win->width * y + x];
            /* draw vertical data */
            for (v = 0; v < (font->height/8); v++)
            {
                for (mask = 0x01; mask < 0x100; mask <<= 1)
                {
                    if (*data & mask)
                        mem->value = color;
                    else
                        mem->value = bgcolor;
                    mem += win->width;
                }
                data++;
            }
            x++;
        }
    }
}

/*
 * put text centered about some point
 */
void _gs_text_put_centered(struct gs_win_t *win,
        char *str, int slen, int x, int y, int color, int bgcolor, struct gs_font_t *font)
{
    int width, height;

    if (!_gs_text_size(str, slen, font, &width, &height))
        return;

    x -= width / 2;
    if (x < 0)
        x = 0;

    y -= height / 2;
    if (y < 0)
        y = 0;

    _gs_text_put(win, str, slen, x, y, color, bgcolor, font);
}

/*
 * get string dimensions with given font
 *
 * RETURN
 *     1 on success, 0 otherwise
 */
int _gs_text_size(char *str, int slen, struct gs_font_t *font, int *width, int *height)
{
    int sl;

    if (!str || !font || !width || !height)
        return 0;

    *height = font->height;
    *width  = font->width;

    sl = strlen(str);
    if (slen > sl)
        slen = sl;

    if (slen)
        *width *= slen;
    else
        *width *= sl;

    return 1;
}


