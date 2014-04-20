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
 *     win     parent window of widget
 *     width   width of pixmap in pixeles
 *     height  height of pixmap in pixeles
 */
struct gs_widget_t *
gs_wpixmap_create(struct gs_win_t *win, int x, int y, int width, int height)
{
    struct gs_widget_t *wd;

    if (!win)
        return NULL;

    wd = gsp_malloc(sizeof(struct gs_widget_t));
    if (!wd)
        return NULL;
    gsp_memset(wd, 0, sizeof(struct gs_widget_t));

    wd->wpixmap.data = gsp_malloc(width * height * GS_PIXEL_DEPTH);
    if (!wd->wpixmap.data)
        goto err_alloc;

    /* generic fields */
    {
        wd->type        = GS_WIDGET_TYPE_PIXMAP;
        wd->destructor  = gs_wpixmap_destroy;
        wd->x0           = x;
        wd->y0           = y;
        wd->x1          = x + width - 1;
        wd->y1          = y + height - 1;
        wd->touch_cb    = NULL;
    }

    /* custom fields */
    {
        wd->wpixmap.width  = width;
        wd->wpixmap.height = height;
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
void
gs_wpixmap_destroy(struct gs_widget_t *wd)
{
    if (!wd || wd->type != GS_WIDGET_TYPE_PIXMAP)
        return;

    gsp_free(wd->wpixmap.data);
    gs_widget_destroy(wd);
}

/*
 * set value of text in text widget
 */
void
gs_wpixmap_set_image(struct gs_widget_t *wd, union gs_pixel_t *image)
{
    if (!wd || wd->type != GS_WIDGET_TYPE_PIXMAP)
        return;

    gsp_memcpy(wd->wpixmap.data, image,
            wd->wpixmap.width * wd->wpixmap.height * GS_PIXEL_DEPTH);
}

/*
 * set alpha channel of specific color on pixmap
 */
void
gs_wpixmap_set_alpha_on_color(struct gs_widget_t *wd, int color, uint8 alpha)
{
    union gs_pixel_t *pix;
    int len;

    if (!wd || wd->type != GS_WIDGET_TYPE_PIXMAP)
        return;

    pix = wd->wpixmap.data;
    len = wd->wpixmap.width * wd->wpixmap.height;
    while (len--)
    {
        if (pix->value == color)
            pix->a = alpha;
        pix++;
    }
}

/*
 * set alpha channel of pixmap
 */
void
gs_wpixmap_set_alpha(struct gs_widget_t *wd, uint8 alpha)
{
    union gs_pixel_t *pix;
    int len;

    if (!wd || wd->type != GS_WIDGET_TYPE_PIXMAP)
        return;

    pix = wd->wpixmap.data;
    len = wd->wpixmap.width * wd->wpixmap.height;
    while (len--)
    {
        pix->a = alpha;
        pix++;
    }
}

/*
 * map each pixel to it's parent window with alpha channel
 *
 * NOTE alpha channel of parent window is not used
 */
void
gs_wpixmap_draw(struct gs_widget_t *wd)
{
    int x, y;
    union gs_pixel_t *mem;
    union gs_pixel_t *pix;
    int r, g, b;

    if (!wd || wd->type != GS_WIDGET_TYPE_PIXMAP || !wd->pwin)
        return;

    pix = wd->wpixmap.data;
    for (y = wd->y0; y <= wd->y1; y++)
    {
        mem = &wd->pwin->mem[wd->pwin->width * y + wd->x0];
#define ALPHA_MAX    255
        for (x = wd->x0; x <= wd->x1; x++)
        {
            r = (pix->red   * pix->a  + mem->red   * (ALPHA_MAX - pix->a)) / ALPHA_MAX;
            g = (pix->green * pix->a  + mem->green * (ALPHA_MAX - pix->a)) / ALPHA_MAX;
            b = (pix->blue  * pix->a  + mem->blue  * (ALPHA_MAX - pix->a)) / ALPHA_MAX;

            mem->red   = r & 0xff;
            mem->green = g & 0xff;
            mem->blue  = b & 0xff;

            mem++;
            pix++;
        }
    }
}

/*
 * Map each pixel to it's parent window. Substitute specific color with background
 * of it's parent window.
 *
 */
void
gs_wpixmap_draw2(struct gs_widget_t *wd, int color)
{
    int x, y;
    union gs_pixel_t *mem;
    union gs_pixel_t *pix;

    if (!wd || wd->type != GS_WIDGET_TYPE_PIXMAP || !wd->pwin)
        return;

    pix = wd->wpixmap.data;
    for (y = wd->y0; y <= wd->y1; y++)
    {
        mem = &wd->pwin->mem[wd->pwin->width * y + wd->x0];
        for (x = wd->x0; x <= wd->x1; x++)
        {
            if ((pix->value & 0x00ffffff) == (color & 0x00ffffff))
                mem->value = wd->pwin->bgcolor.value;
            else
                mem->value = pix->value;

            mem++;
            pix++;
        }
    }
}

/*
 * fill pixmap area with window background color
 */
void
gs_wpixmap_clear(struct gs_widget_t *wd)
{
    int x, y;
    union gs_pixel_t *mem;

    if (!wd || wd->type != GS_WIDGET_TYPE_PIXMAP || !wd->pwin)
        return;

    for (y = wd->y0; y <= wd->y1; y++)
    {
        mem = &wd->pwin->mem[wd->pwin->width * y + wd->x0];
#define ALPHA_MAX    255
        for (x = wd->x0; x <= wd->x1; x++)
        {
            mem->value = wd->pwin->bgcolor.value;
            mem++;
        }
    }
}

