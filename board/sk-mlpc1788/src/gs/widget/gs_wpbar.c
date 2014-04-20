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
 *     width  horizontal dimension in pixels
 *     height vertical dimension in pixels
 */
struct gs_widget_t *gs_wpbar_create(struct gs_win_t *win,
    int x, int y, int width, int height)
{
    struct gs_widget_t *wd;

    wd = gsp_malloc(sizeof(struct gs_widget_t));
    if (!wd)
        return NULL;
    gsp_memset(wd, 0, sizeof(struct gs_widget_t));

    /* sanity check */
    if ((x + width) >= win->width)
        goto error;
    if ((y + height) >= win->height)
        goto error;

    /* generic fields */
    {
        wd->type        = GS_WIDGET_TYPE_PBAR;
        wd->destructor  = gs_widget_destroy;
        wd->x0           = x;
        wd->y0           = y;
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
 * set colors of progress bar
 */
void gs_wpbar_set_colors(struct gs_widget_t *wd, int fg, int bg, int b)
{
    if (!wd || wd->type != GS_WIDGET_TYPE_PBAR)
        return;

    wd->wpbar.fg_color.value = fg;
    wd->wpbar.bg_color.value = bg;
    wd->wpbar.br_color.value  = b;
}

/*
 * set value of widget
 */
void gs_wpbar_set_value(struct gs_widget_t *wd, int value)
{
    if (!wd || wd->type != GS_WIDGET_TYPE_PBAR)
        return;

    wd->wpbar.value = value;
}

/*
 * draw progress bar to it's parent window
 */
void gs_wpbar_draw(struct gs_widget_t *wd)
{
    int x0, y0, x1, y1;

    struct gs_win_t *win;

    if (!wd || !wd->pwin || wd->type != GS_WIDGET_TYPE_PBAR)
        return;
    win = wd->pwin;

    /* sanity check */
    if (wd->wpbar.value < 0 || wd->wpbar.value > 100)
        return;

    x0 = wd->x0;
    y0 = wd->y0;
    x1 = wd->x0 + (wd->x1 - wd->x0) * wd->wpbar.value / 100;
    y1 = wd->y1;
    if (x1 > x0 && y1 > y0)
        gs_prim_fill_rect(win, x0, y0, x1, y1, wd->wpbar.fg_color.value);

    x0 = wd->x1 - (wd->x1 - wd->x0) * (100 - wd->wpbar.value) / 100;
    x1 = wd->x1;
    if (x1 > x0 && y1 > y0)
        gs_prim_fill_rect(win, x0, y0, x1, y1, wd->wpbar.bg_color.value);

    gs_prim_draw_rect(win, wd->x0, wd->y0, wd->x1, wd->y1, 1, wd->wpbar.br_color.value);
}


