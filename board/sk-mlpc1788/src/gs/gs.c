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
#include <os.h>
#include <stimer.h>
#include "gs.h"
#include "gsp.h"


/*
 * NOTE
 * Base address should be doubleword aligned. First buffer is aligned by linker script.
 * Check that second buffer also alligned. If not then add additional pixels.
 */
union gs_pixel_t gs_mem[GS_MEM_BUFNUM][GS_PIXELNUM] __attribute((section("gsmem")));
#ifdef GS_SINGLE_WINDOW
    struct gs_win_t single_window;
#else
    static void gs_map_windows();
    static void gs_add_win(struct gs_win_t *win);
    static void gs_remove_win(struct gs_win_t *win);
#endif

/*
 *
 */
void gs_task()
{
    union gs_pixel_t *pixel;
    int i, n;

    /* fill root window with background */
    for (i = 0; i < GS_MEM_BUFNUM; i++)
    {
        pixel = gs_mem[i];
        n = GS_PIXELNUM;
        while (n--)
        {
            pixel->value = GS_ROOTWIN_DEFAULT_COLOR;
            pixel++;
        }
    }
    gsp_hw_init();

#ifdef GS_SINGLE_WINDOW
    gsp_memset(&single_window, 0, sizeof(struct gs_win_t));
    
    single_window.x             = 0;
    single_window.y             = 0;
    single_window.width         = GS_RES_HORIZONTAL;
    single_window.height        = GS_RES_VERTICAL;
    single_window.bgcolor.value = GS_ROOTWIN_DEFAULT_COLOR;
    single_window.bwidth        = 0;
    single_window.bcolor.value  = 0;
    single_window.mem           = gs_inactive_buf;
    single_window.widgets       = 0;
#endif

    gs_font_init();
    gsp_init_end();

    while (1)
    {
#ifdef GS_SINGLE_WINDOW
        os_wait(OS_WAIT_FOREVER); /* XXX not necessary to create task actually */
#else
        gs_map_windows();
#endif
    }
}

#ifndef GS_SINGLE_WINDOW
#ifndef GS_SIMPLE_WM
/*
 * check where window cross pane
 *
 * ARGS
 *     win    pointer to window
 *     cross  destination pane, contain shared area of source pane and window
 *     pane   source pane
 *
 *
 * RETURN
 *     0 if window does not cross pane
 */
int gs_win_cross_pane(struct gs_win_t *win, struct gs_pane_t *cross, struct gs_pane_t *pane)
{
    int crossed_x, crossed_y;
    if (!win)
        return 0;

    if (pane->width == 0 || pane->height == 0)
        return 0;

    crossed_x = 0;
    crossed_y = 0;
    if ((win->x < pane->x) && ((win->x + win->width) > pane->x))
    {
        crossed_x = 1;
        cross->x = pane->x;
        if ((win->x + win->width) > (pane->x + pane->width))
            cross->width = pane->width;
        else
            cross->width = win->x + win->width - pane->x;
    } else if ((win->x >= pane->x) && (win->x < (pane->x + pane->width))) {
        crossed_x = 1;
        cross->x = win->x;
        if ((win->x + win->width) > (pane->x + pane->width))
            cross->width = pane->x + pane->width - win->x;
        else
            cross->width = win->width;
    }

    if ((win->y < pane->y) && ((win->y + win->height) > pane->y))
    {
        crossed_y = 1;
        cross->y = pane->y;
        if ((win->y + win->height) > (pane->y + pane->height))
            cross->height = pane->height;
        else
            cross->height = win->y + win->height - pane->y;
    } else if ((win->y >= pane->y) && (win->y < (pane->y + pane->height))) {
        crossed_y = 1;
        cross->y = win->y;
        if ((win->y + win->height) > (pane->y + pane->height))
            cross->height = pane->y + pane->height - win->y;
        else
            cross->height = win->height;
    }

    if (crossed_x && crossed_y)
        return 1;

    return 0;
}

/*
 *
 * ARGS
 *     win     start window
 *     pane    pane to map window
 *
 * NOTE function is called recursively - be care that stack size is large enough
 */
static void gs_refresh_stack(struct gs_win_t *win, struct gs_pane_t *pane)
{
    struct gs_pane_t *cross, cpane;
    struct gs_pane_t npane[4];

    if (!win)
        return;

    cross = &cpane;

    /* if window cross pane then refresh crossing part */
    if (gs_win_cross_pane(win, cross, pane))
    {
        if (win->map)
            gsp_win_map2(win, gs_inactive_buf, cross);

        /*
         * compose other four panes for next window in stack
         *
         * pane
         * +-------+----------+-------+
         * |       |  pane1   |       |
         * |       +----------+       |
         * | pane0 | cross    | pane3 |
         * |       +----------+       |
         * |       |  pane2   |       |
         * +-------+----------+-------+
         */
        if (win->next)
        {
            npane[0].x      = pane->x;
            npane[0].y      = pane->y;
            npane[0].width  = cross->x - pane->x;
            npane[0].height = pane->height;

            npane[1].x      = cross->x;
            npane[1].y      = pane->y;
            npane[1].width  = cross->width;
            npane[1].height = cross->y - pane->y;

            npane[2].x      = cross->x;
            npane[2].y      = cross->y + cross->height;
            npane[2].width  = cross->width;
            npane[2].height = pane->height - (npane[1].height + cross->height);

            npane[3].x      = cross->x + cross->width;
            npane[3].y      = pane->y;
            npane[3].width  = pane->width - (npane[0].width + cross->width);
            npane[3].height = pane->height;

            gs_refresh_stack(win->next, &npane[0]);
            gs_refresh_stack(win->next, &npane[1]);
            gs_refresh_stack(win->next, &npane[2]);
            gs_refresh_stack(win->next, &npane[3]);
        }
    } else {
        if (win->next)
            gs_refresh_stack(win->next, pane);
    }
}

/*
 * map all windows
 */
static void gs_map_windows()
{
    struct gs_win_t *win;
    struct gs_pane_t pane;
    int refresh;

    refresh = 0;
    gsp_mutex_lock();
    {
        pane.x      = 0;
        pane.y      = 0;
        pane.width  = GS_RES_HORIZONTAL;
        pane.height = GS_RES_VERTICAL;

        for (win = gsp.wmhead; win != NULL; win = win->next)
        {
            if (win->map)
                refresh = 1;
        }

        /* loop windows stack */
        if (refresh)
            gs_refresh_stack(gsp.wmhead, &pane);

        /* decrement map counter */
        for (win = gsp.wmhead; win != NULL; win = win->next)
        {
            if (win->map)
            {
                win->map--;
                if (win->map == 0)
                    gsp_win_refresh_finish(win);
            }
        }
    }
    gsp_mutex_unlock();

    gsp_wait_vsync();
    if (refresh)
    {
        gs_switch_abuf();
    }
}
#else /* GS_SIMPLE_WM */
/*
 * map all windows
 */
static void gs_map_windows()
{
    struct gs_win_t *win;
    int switchbuf;

    switchbuf = 0;
    gsp_mutex_lock();
    {
        /* wmhead contain bottom window in stack */
        for (win = gsp.wmhead; win != NULL; win = win->next)
        {
            if (win->map)
            {
                gsp_win_map(win, gs_inactive_buf);
                switchbuf = 1;

                win->map--;
                if (win->map == 0)
                    gsp_win_refresh_finish(win);
            }
        }
    }
    gsp_mutex_unlock();

    gsp_wait_vsync();
    if (switchbuf)
        gs_switch_abuf();
}
#endif /* GS_SIMPLE_WM */
#endif /* GS_SINGLE_WINDOW */

/*
 * switch active buffer
 */
void gs_switch_abuf()
{
    gsp.abuf &= 0x01;
    gsp.abuf ^= 0x01;
    gsp_set_active_buffer(gs_active_buf);
}

#ifdef GS_SINGLE_WINDOW
/*
 *
 */
struct gs_win_t *gs_win_create(int x, int y, int width, int height)
{
    return &single_window;
}

/*
 *
 */
void gs_win_destroy(struct gs_win_t *win)
{
    return;
}
#else

/*
 *
 */
struct gs_win_t *gs_win_create(int x, int y, int width, int height)
{
    struct gs_win_t *win;
    union gs_pixel_t *pixel;
    int pixnumber;

    /* sanity check of dimensions */
    if ((x + width) > GS_RES_HORIZONTAL ||
        (y + height) > GS_RES_VERTICAL)
        return NULL;

    win = gsp_malloc(sizeof(struct gs_win_t));
    gsp_memset(win, 0, sizeof(struct gs_win_t));

    pixnumber    = width * height;
    win->mem     = gsp_malloc(pixnumber * GS_PIXEL_DEPTH);
    win->width   = width;
    win->height  = height;
    win->x       = x;
    win->y       = y;

    /* fill window with default background */
    pixel = win->mem;
    while (pixnumber--)
    {
        pixel->value = GS_WIN_DEFAULT_COLOR;
        pixel++;
    }
    win->bgcolor.value = GS_WIN_DEFAULT_COLOR;

    gsp_mutex_lock();
    gs_add_win(win);
    gsp_mutex_unlock();

    return win;
}

/*
 *
 */
void gs_win_destroy(struct gs_win_t *win)
{
    int i;
    struct gs_widget_t *wnext;

    if (!win)
        return;

    gsp_mutex_lock();
    {
        /* destroy widgets */
        for (;win->widgets != NULL; win->widgets = wnext)
        {
            wnext = win->widgets->next;
            (*win->widgets->destructor)(win->widgets);
        }

        /* set background of window to background of root window */
        for (i = 0; i < (win->width * win->height); i++)
            win->mem[i].value = GS_ROOTWIN_DEFAULT_COLOR;

        /* copy window to main memory */
        gsp_win_map(win, gs_inactive_buf);

        gsp_wait_vsync();
        gs_switch_abuf();

        gsp_win_map(win, gs_inactive_buf);

        /* remove window from list */
        gs_remove_win(win);
        gsp_free(win->mem);
        gsp_free(win);
    }
    gsp_mutex_unlock();
}

/*
 * raise window on stack
 *     c    count of stack levels to move window
 *
 * XXX
 *     not tested
 */
void gs_win_raise(struct gs_win_t *win, int c)
{
    int i;
    struct gs_win_t *pwin, *nwin;

    if (!win)
        return;

    pwin = NULL;
    nwin = NULL;
    gsp_mutex_lock();
    {
        for (i = 0; i < c && win->next; i++)
        {
            if (win->prev)
                pwin = win->prev;
            else
                if (win->next)
                    gsp.wmhead = win->next;

            /* NOTE circle should not be entered on win->next == NULL */
            nwin = win->next;

            if (pwin)
                pwin->next = nwin;

            if (win->prev)
                win->prev  = nwin;
            if (win->next)
                win->next = nwin->next;

            if (win->prev)
                nwin->prev = win->prev;
            nwin->next = win;
        }
    }
    gsp_mutex_unlock();

    /* TODO refresh each window on stack */
}

/*
 * add window to list of windows managed by GS
 */
static void gs_add_win(struct gs_win_t *win)
{
    struct gs_win_t *pw;

    if (!win)
        return;

    if (gsp.wmhead == NULL)
    {
        gsp.wmhead = win;
        win->prev  = NULL;
        win->next  = NULL;
    } else {
        for (pw = gsp.wmhead; pw->next != NULL; pw = pw->next)
            ;
        pw->next  = win;
        win->prev = pw;
        win->next = NULL;
    }
}

/*
 * remove window from list of windows managed by GS
 */
static void gs_remove_win(struct gs_win_t *win)
{
    if (!win)
        return;

    if (win == gsp.wmhead)
        gsp.wmhead = win->next;

    if (win->prev)
        win->prev->next = win->next;
    if (win->next)
        win->next->prev = win->prev;
}
#endif /* GS_SINGLE_WINDOW */

/*
 *
 */
void gs_win_set_border(struct gs_win_t *win, int width, int color)
{
    int y, x;

    if (!win)
        return;

    win->bwidth = width;
    win->bcolor.value = color;

    /* upper and bottom border */
    for (y = 0; y < width; y++)
    {
        for (x = 0; x < win->width; x++)
        {
            win->mem[win->width * y +                     x].value = color;
            win->mem[win->width * (win->height - 1 - y) + x].value = color;
        }
    }

    /* left and right border */
    for (x = 0; x < width; x++)
    {
        for (y = width; y < win->height - width; y++)
        {
            win->mem[win->width * y + x].value = color;
            win->mem[win->width * y + (win->width - 1 - x)].value = color;
        }
    }
}

/*
 * fill window with default background
 */
void gs_win_clear(struct gs_win_t *win)
{
    int y, c, lines, pixels;
    union gs_pixel_t *mem;

    mem    = &win->mem[win->bwidth * win->width + win->bwidth];
    lines  = win->height - (2 * win->bwidth);
    pixels = win->width  - (2 * win->bwidth);
    for (y = win->bwidth; y < lines; y++)
    {
        c = pixels;
        while (c--)
        {
            mem->value = win->bgcolor.value;
            mem++;
        }
        mem += win->bwidth;
    }
}

/*
 * fill window with back-plane
 */
void gs_win_hide(struct gs_win_t *win)
{
    gsp_mutex_lock();
    {
        /* TODO */
    }
    gsp_mutex_unlock();
}

/*
 *
 */
inline void gs_win_refresh(struct gs_win_t *win)
{
    if (!win)
        return;

    gsp_win_refresh(win);
}

#ifdef GS_SINGLE_WINDOW
/*
 *
 */
inline void gs_win_refresh_widgets(struct gs_win_t *win)
{
    if (!win)
        return;

    gsp_win_refresh_widgets(win);
}
#endif

/*
 *
 */
inline int gs_initialized()
{
    return gsp_initialized();
}

/*
 *
 */
void gs_win_widget_add(struct gs_win_t *win, struct gs_widget_t *widget)
{
    struct gs_widget_t *wd;

    if (!win)
        return;

    gsp_mutex_lock();
    {
        if (win->widgets == NULL)
        {
            win->widgets = widget;
            widget->prev = NULL;
            widget->next = NULL;
            widget->pwin = win;
            gsp_mutex_unlock();
            return;
        }

        for (wd = win->widgets; wd->next != NULL; wd = wd->next)
            ;

        wd->next = widget;
        widget->prev = wd;
        widget->next = NULL;
        widget->pwin = win;
    }
    gsp_mutex_unlock();
}

/*
 *
 */
void gs_win_widget_remove(struct gs_win_t *win, struct gs_widget_t *widget)
{
    if (!win)
        return;

    gsp_mutex_lock();
    {
        if (win->widgets == widget)
        {
            win->widgets = win->widgets->next;
        } else {
            if (widget->prev)
                widget->prev->next = widget->next;
            if (widget->next)
                widget->next->prev = widget->prev;
        }

        widget->pwin = NULL;
    }
    gsp_mutex_unlock();
}

///*
// *
// */
//void gs_touch(int x, int y)
//{
//    struct gs_win_t *win;
//    struct gs_widget_t *wd;
//
//    gsp_mutex_lock();
//    {
//        for (win = gsp.wmhead; win != NULL; win = win->next)
//        {
//            if (x >= win->x && x < (win->x + win->width) &&
//                y >= win->y && y < (win->y + win->height))
//            {
//                /* loop thru widgets */
//                for (wd = win->widgets; wd != NULL; wd = wd->next)
//                {
//                    if (x >= (win->x + wd->x) && x <= (win->x + wd->x1) &&
//                        y >= (win->y + wd->y) && y <= (win->y + wd->y1) )
//                    {
//                        if (wd->touch_cb)
//                            (*wd->touch_cb)(wd, x - win->x, y - win->y);
//                    }
//                }
//                /* TODO no one widget was touched, perform action for window touch */
//                break;
//            }
//        }
//    }
//    gsp_mutex_unlock();
//}
//
