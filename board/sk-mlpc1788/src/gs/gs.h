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

#ifndef GS_H
#define GS_H

#include <types.h>

#define GS_PIXEL_DEPTH        4   /* 24 bpp */
#define GS_RES_HORIZONTAL     480
#define GS_RES_VERTICAL       272

#define GS_PIXELNUM (GS_RES_HORIZONTAL * GS_RES_VERTICAL)
#define GS_MEM_SIZE     (GS_PIXELNUM * GS_PIXEL_DEPTH)

//#define GS_WIN_DEFAULT_COLOR          0xFF336611
//#define GS_WIN_DEFAULT_COLOR          0xff402020
#define GS_WIN_DEFAULT_COLOR          0xff202520
#define GS_ROOTWIN_DEFAULT_COLOR      GS_WIN_DEFAULT_COLOR
#define GS_WIN_BORDER_DEFAULT_COLOR   0xFFFFFFFF

#define GS_COLOR_WHITE    0xFFFFFFFF
#define GS_COLOR_YELLOW   0xFF00FFFF

#define GS_SINGLE_WINDOW
//#define GS_SIMPLE_WM

#define GS_RGB2COLOR(r, g, b) ((b << 16) | (g << 8) | r)
union gs_pixel_t {
#if (GS_PIXEL_DEPTH == 4)
    uint32 value;
    struct {
        uint8 red;
        uint8 green;
        uint8 blue;
        uint8 a; /* NOTE unused */
    };
#elif (GS_PIXEL_DEPTH == 1)
    uint8 value;
#else
    #error Unknown pixel size
#endif
} __attribute__((packed));

struct gs_pane_t {
    int x;
    int y;
    int width;
    int height;
};

struct gs_win_t {
    int x;                       /* current X coordinate of window */
    int y;                       /* current Y coordinate of window */
    int width;                   /* width of window */
    int height;                  /* height of window */
    union gs_pixel_t bgcolor;    /* background color of window */
    int bwidth;                  /* border line width */
    union gs_pixel_t bcolor;     /* border color of window */

    union gs_pixel_t *mem;       /* drawing area of window */
    struct gs_widget_t *widgets; /* linked lists off widgets attached to window */

#define GS_WIN_EVENT_MASK_REFRESH    (1 << 0)
    BASE_TYPE events;            /* miscellaneous events */

//#define GS_WIN_FLAG_ALWAYS_ON_TOP (1 << 0)
//    int flags;                   /* miscellaneous flags */

#ifndef GS_SINGLE_WINDOW
    int map;                     /* map counter */
    /* for linked list of windows managed by GS */
    struct gs_win_t *prev;
    struct gs_win_t *next;
#endif
};

#define GS_MEM_BUFNUM    2 /* double buffered memory */
extern union gs_pixel_t gs_mem[GS_MEM_BUFNUM][GS_PIXELNUM];

#ifdef GS_SINGLE_WINDOW
    extern struct gs_win_t single_window;
#endif

void gs_task();
int gs_initialized();
struct gs_win_t *gs_win_create(int x, int y, int width, int height);
void gs_win_destroy(struct gs_win_t *win);
void gs_win_refresh(struct gs_win_t *win);
#ifdef GS_SINGLE_WINDOW
void gs_win_refresh_widgets(struct gs_win_t *win);
#endif
void gs_win_set_border(struct gs_win_t *win, int width, int color);
void gs_win_clear(struct gs_win_t *win);
void gs_win_widget_add(struct gs_win_t *win, struct gs_widget_t *widget);
void gs_win_widget_remove(struct gs_win_t *win, struct gs_widget_t *widget);
void gs_switch_abuf();

#define GS_WIN_RAISE_TOP       0xffffffff
#define GS_WIN_RAISE_BOTTOM    0xffffffff
void gs_win_raise(struct gs_win_t *win, int dir);
void gs_win_hide(struct gs_win_t *win);
//void gs_touch(int x, int y);

#define gs_active_buf (gs_mem[gsp.abuf])
#define gs_inactive_buf (gs_mem[gsp.abuf ^ 0x01])

#include "gs_font.h"
#include "gs_text.h"
#include "gs_image.h"
#include "gs_prim.h"
#include "gs_util.h"
#include "gs_widget.h"
#include "widget/gs_wlist.h"
#include "widget/gs_wpbar.h"
#include "widget/gs_wtext.h"
#include "widget/gs_wpixmap.h"
#include "widget/gs_wvolume.h"

#endif

