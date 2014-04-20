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
#include <debug.h>
#include "../gs.h"
#include "../gsp.h"

/*
 * create text widget
 *
 * ARGS
 *     win    parent window of widget
 *     x      X coordinate of widget inside window
 *     y      Y coordinate of widget inside window
 *     font   font used to print text
 *     slen   maximum string length of widget
 */
struct gs_widget_t *gs_wtext_create(struct gs_win_t *win,
    int x, int y, struct gs_font_t *font, int slen)
{
    struct gs_widget_t *wd;

    if (!win || !font)
        return NULL;
    if (x + (font->width * slen) >= win->width)
        return NULL;
    if (y + (font->height) >= win->height)
        return NULL;

    wd = gsp_malloc(sizeof(struct gs_widget_t));
    if (!wd)
        return NULL;
    gsp_memset(wd, 0, sizeof(struct gs_widget_t));

    wd->wtext.s = gsp_malloc(slen + 1); /* NOTE +1 for null terminator */
    if (!wd->wtext.s)
        goto err_alloc;

    wd->wtext.s[0] = 0;

    /* generic fields */
    {
        wd->type        = GS_WIDGET_TYPE_TEXT;
        wd->destructor  = gs_wtext_destroy;
        wd->x0           = x;
        wd->y0           = y;
        wd->x1          = x + font->width * slen;
        wd->y1          = y + font->height;
        wd->touch_cb    = NULL;
    }

    /* custom fields */
    {
        wd->wtext.maxslen = slen;
        wd->wtext.font    = font;
        wd->wtext.fg_color.value = 0xffffff;
        wd->wtext.bg_color.value = 0x000000;
    }

    gs_win_widget_add(win, wd);
    return wd;
err_alloc:
    gsp_free(wd);
    return NULL;
}

/*
 *
 */
void gs_wtext_destroy(struct gs_widget_t *wd)
{
    if (!wd || wd->type != GS_WIDGET_TYPE_TEXT)
        return;

    gsp_free(wd->wtext.s);
    gs_widget_destroy(wd);
}

/*
 * set colors of text widget
 */
void gs_wtext_set_colors(struct gs_widget_t *wd, int fg, int bg)
{
    if (!wd || wd->type != GS_WIDGET_TYPE_TEXT)
        return;

    wd->wtext.fg_color.value = fg;
    wd->wtext.bg_color.value = bg;
}

/*
 * set value of text in text widget
 */
void gs_wtext_set_text(struct gs_widget_t *wd, char *s)
{
    if (!wd || wd->type != GS_WIDGET_TYPE_TEXT)
        return;

    if (strlen(s) > wd->wtext.maxslen)
    {
        strncpy(wd->wtext.s, s, wd->wtext.maxslen);
        wd->wtext.s[wd->wtext.maxslen] = 0;
    } else {
        strcpy(wd->wtext.s, s);
    }
}

/*
 *
 */
void gs_wtext_draw(struct gs_widget_t *wd, int align)
{
    if (!wd || wd->type != GS_WIDGET_TYPE_TEXT || !wd->pwin)
        return;

    /* XXX clear */
    gs_prim_fill_rect(wd->pwin, wd->x0, wd->y0, wd->x1, wd->y1, wd->wtext.bg_color.value);

    if (align == GS_WTEXT_DRAW_CENTERED)
        gs_text_put_centered(wd->pwin, wd->wtext.s,
                (wd->x0 + wd->x1) / 2, (wd->y0 + wd->y1) / 2,
                wd->wtext.fg_color.value, wd->wtext.bg_color.value, wd->wtext.font);
    else
        gs_text_put(wd->pwin, wd->wtext.s,
                wd->x0, wd->y0,
                wd->wtext.fg_color.value, wd->wtext.bg_color.value, wd->wtext.font);

}

