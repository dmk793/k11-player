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

#ifndef _GS_WIDGET_H_
#define _GS_WIDGET_H_

#include "gs_font.h"
#include "gs.h"

struct gs_widget_t {
#define GS_WIDGET_TYPE_BUTTON    0x01
#define GS_WIDGET_TYPE_LIST      0x02
#define GS_WIDGET_TYPE_PBAR      0x03
#define GS_WIDGET_TYPE_TEXT      0x04
#define GS_WIDGET_TYPE_PIXMAP    0x05
#define GS_WIDGET_TYPE_VOLUME    0x06
    int type;               /* type of widget */
    union {
        struct {
            struct gs_wlist_entry_t {
                char *st0;     /* value to display */
                char *st1;     /* real value */
                uint32 hash1;  /* hash of st1 */
            } *lentry;
            struct gs_wlist_entry_t *bsort; /* buffer for sorting */
//            char **entry;
            int maxslen; /* maximum string length of entry */
            int width;   /* width in pixeles */
            int height;  /* height in pixeles */
            int nentr;   /* simultaenous visible entries */
            int num;     /* overall number of entries */
            int idx;     /* index of first visible entry */
            int act;     /* index of last active entry */
            int sel;     /* index of selected entry */
            int cur;     /* index of entry with cursor */

            union gs_pixel_t font_color;     /* foreground font color */
            union gs_pixel_t sel_color;      /* background color of selected entry */
            union gs_pixel_t cur_color;      /* cursor color */
            union gs_pixel_t curedge_color;  /* cursor color of first/last entry */
            struct gs_font_t *font;

            /* used only by list with images */
            union gs_pixel_t **image;
            int iwidth;
            int iheight;
        } wlist;

        struct {
            int   value;               /* current value in percents */
            union gs_pixel_t fg_color; /* foreground color */
            union gs_pixel_t bg_color; /* background color */
            union gs_pixel_t br_color; /* border color */
        } wpbar;

        struct {
            char *s;
            int maxslen;
            struct gs_font_t *font;
            union gs_pixel_t fg_color; /* foreground font color */
            union gs_pixel_t bg_color; /* background font color */
        } wtext;

        struct {
            union gs_pixel_t *data; /* data of pixmap */
            int width;              /* width in pixeles */
            int height;             /* height in pexeles */
        } wpixmap;

        struct {
            int height; /* height of volume widget */
            int nbars;  /* number of bars */
            int barw;   /* width of one bar */
            int gapw;   /* gap between bars */
            int value;  /* value between 0 and 100 */

            union gs_pixel_t fg_color;  /* selected color */
            union gs_pixel_t bg_color;  /* unselected color */
            union gs_pixel_t br_color;  /* border color */
        } wvolume;
    };

    int x0;                 /* X coordinate of widget relative to parent window */
    int y0;                 /* Y coordinate of widget relative to parent window */
    int x1;                 /* X coordinate of right corner */
    int y1;                 /* Y coordinate of top corner */

    struct gs_win_t *pwin;  /* parent window */

#ifdef GS_SINGLE_WINDOW
    /*
     * Now map of specific widget supported only if single window
     * algorithm is used.
     */
    int map;
#endif
    void (*touch_cb)(struct gs_widget_t *, int x, int y); 
    void (*destructor)(struct gs_widget_t *);

    /* for linked list of widgets contained by window */
    struct gs_widget_t *next;
    struct gs_widget_t *prev;
};

void gs_widget_destroy(struct gs_widget_t *wd);
#ifdef GS_SINGLE_WINDOW
void gs_widget_refresh(struct gs_widget_t *wd);
#endif

#endif /* !_GS_WIDGET_H_ */

