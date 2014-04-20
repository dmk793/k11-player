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
#include "../gs.h"
#include "../gsp.h"

/*
 * create progress bar widget
 *
 * ARGS
 *     win    parent window of widget
 *     x      X coordinate of widget inside window
 *     y      Y coordinate of widget inside window
 *     nbars  number of bars in volume widget
 *     barw   width of one bar
 *     gapw   gap width between bars
 */
struct gs_widget_t *
gs_wvolume_create(struct gs_win_t *win,
    int x, int y, int height, int nbars, int barw, int gapw)
{
    struct gs_widget_t *wd;
    int width;

    wd = gsp_malloc(sizeof(struct gs_widget_t));
    if (!wd)
        return NULL;
    gsp_memset(wd, 0, sizeof(struct gs_widget_t));

    width = (barw + gapw) * nbars;

    /* sanity check */
    if ((x + width) >= win->width)
        goto error;
    if ((y + height) >= win->height)
        goto error;

    /* custom fields */
    {
        wd->wvolume.height = height;
        wd->wvolume.nbars  = nbars;
        wd->wvolume.barw   = barw;
        wd->wvolume.gapw   = gapw;
    }

    /* generic fields */
    {
        wd->type        = GS_WIDGET_TYPE_VOLUME;
        wd->destructor  = gs_widget_destroy;
        wd->x0          = x;
        wd->y0          = y;
        wd->x1          = x + width  - 1;
        wd->y1          = y + height - 1;
        wd->touch_cb    = NULL;
    }

    gs_win_widget_add(win, wd);
    return wd;
error:
    gsp_free(wd);
    return NULL;
}

/*
 * set colors of volume widget
 * ARGS
 *     wd     pointer to widget
 *     fg     foreground color
 *     bg     background color
 *     br     border     color
 */
void
gs_wvolume_set_colors(struct gs_widget_t *wd, int fg, int bg, int br)
{
    if (!wd || wd->type != GS_WIDGET_TYPE_VOLUME)
        return;

    wd->wvolume.fg_color.value = fg;
    wd->wvolume.bg_color.value = bg;
    wd->wvolume.br_color.value = br;
}


/*
 * set value of widget
 */
void gs_wvolume_set_value(struct gs_widget_t *wd, int value)
{
    if (!wd || wd->type != GS_WIDGET_TYPE_VOLUME)
        return;

    if (value < 0)
        value = 0;
    if (value > 100)
        value = 100;

    wd->wvolume.value = value;
}

/*
 * draw widget to it's parent window
 */
void gs_wvolume_draw(struct gs_widget_t *wd)
{
    int nbars;
    int dh;
    int x0, y0, x1, y1;
    int height;
    int thresh;
    int n;

    if (!wd || wd->type != GS_WIDGET_TYPE_VOLUME || !wd->pwin)
        return;

#define MIN_HEIGHT   4

    /* compute bar height decrement */
    dh = (wd->wvolume.height - MIN_HEIGHT) / wd->wvolume.nbars;
    if (dh < 1)
        dh = 1;

    nbars  = wd->wvolume.nbars;
    height = wd->wvolume.height;
    x0     = wd->x0 + nbars * (wd->wvolume.barw + wd->wvolume.gapw);
    y1     = wd->y1;
    thresh = (100 - wd->wvolume.value) * wd->wvolume.nbars / 100;
    n      = 0;

    while (nbars--)
    {
        x0 -= wd->wvolume.gapw + wd->wvolume.barw;
        x1  = x0 + wd->wvolume.barw - 1;
        y0  = y1 - height + 1;

        if (n < thresh)
            gs_prim_fill_rect(wd->pwin, x0, y0, x1, y1, wd->wvolume.fg_color.value);
        else
            gs_prim_fill_rect(wd->pwin, x0, y0, x1, y1, wd->wvolume.bg_color.value);
        gs_prim_draw_rect(wd->pwin, x0, y0, x1, y1, 1, wd->wvolume.br_color.value);

        if (height > (MIN_HEIGHT + dh))
            height -= dh;
        else
            height = MIN_HEIGHT;

        n++;
    }
}

