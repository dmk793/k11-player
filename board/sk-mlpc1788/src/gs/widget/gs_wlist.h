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

#ifndef GS_WLIST_H
#define GS_WLIST_H

#include <types.h>
#include "../gs.h"

struct gs_wlist_init_info_t {
    struct gs_font_t *font;      /* font used to print text */
    int x;                       /* X coordinate of widget relative to parent window */
    int y;                       /* Y coordinate of widget relative to parent window */
    int width;                   /* width in pixeles */
    int height;                  /* height in pixeles */
    int num;                     /* overall number of entries */
    int maxslen;                 /* maximum string length of entry */

    int iwidth;                  /* width of image, zero if image is not used */
    int iheight;                 /* height of image, zero if image is not used */
    union gs_pixel_t *defimage;  /* default image */
};

struct gs_widget_t *gs_wlist_create(struct gs_win_t *win,
    struct gs_wlist_init_info_t *info);
void gs_wlist_destroy(struct gs_widget_t *wd);
void gs_wlist_set_colors(struct gs_widget_t *wd, int font, int sel, int cur, int cur_edge);
void gs_wlist_draw(struct gs_widget_t *wd);

void gs_wlist_set_st0(struct gs_widget_t *wd, int idx, char *st);
void gs_wlist_set_st1(struct gs_widget_t *wd, int idx, char *st);
char* gs_wlist_get_st0(struct gs_widget_t *wd, int idx);
char* gs_wlist_get_st1(struct gs_widget_t *wd, int idx);
#define GS_WLIST_GET_CUR_ST0(wd) gs_wlist_get_st0(wd, wd->wlist.cur)
#define GS_WLIST_GET_SEL_ST0(wd) gs_wlist_get_st0(wd, wd->wlist.sel)
#define GS_WLIST_GET_CUR_ST1(wd) gs_wlist_get_st1(wd, wd->wlist.cur)
#define GS_WLIST_GET_SEL_ST1(wd) gs_wlist_get_st1(wd, wd->wlist.sel)
int gs_wlist_search_hash1(struct gs_widget_t *wd, uint32 hash);

void gs_wlist_set_image(struct gs_widget_t *wd, int idx, union gs_pixel_t *image);
void gs_wlist_set_active(struct gs_widget_t *wd, int idx);
#define GS_WLIST_SET_SEL_PREV    0
#define GS_WLIST_SET_SEL_NEXT    1
#define GS_WLIST_SET_SEL_VALUE   2
void gs_wlist_set_sel(struct gs_widget_t *wd, int dir, int sel);
#define GS_WLIST_SET_CUR_PREV    0
#define GS_WLIST_SET_CUR_NEXT    1
#define GS_WLIST_SET_CUR_VALUE   2
void gs_wlist_set_cur(struct gs_widget_t *wd, int dir, int cur);
void gs_wlist_sort(struct gs_widget_t *wd);


#define GS_WLIST_SCROLL_UP       0
#define GS_WLIST_SCROLL_DOWN     1
void gs_wlist_scroll(struct gs_widget_t *wd, int dir);

#endif

