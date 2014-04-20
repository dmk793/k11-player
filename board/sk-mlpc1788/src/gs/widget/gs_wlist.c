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
#include <pearson.h>
#include "../gs.h"
#include "../gsp.h"

#define WLIST_IMAGE_PAD    8

/*
 * ARGS
 *     win    parent window
 *     font   font used to print entries
 *     x      X coordinate of widget inside window
 *     y      Y coordinate of widget inside window
 *     width  horizontal dimension in pixels
 *     height vertical dimension in pixels
 *     num    number of entries in list
 *     slen   maximum size of string entry
 *
 * RETURN
 *     pointer to widget structure, NULL otherwise
 */
struct gs_widget_t *gs_wlist_create(struct gs_win_t *win,
    struct gs_wlist_init_info_t *info)
{
    struct gs_widget_t *wd;
    int i;

    if (!win || !info->font || info->x >= win->width || info->y >= win->height)
        return NULL;

    wd = gsp_malloc(sizeof(struct gs_widget_t));
    if (!wd)
        return NULL;
    gsp_memset(wd, 0, sizeof(struct gs_widget_t));

    wd->wlist.lentry = gsp_malloc(sizeof(struct gs_wlist_entry_t) * info->num);
    if (!wd->wlist.lentry)
        goto err_alloc0;
    gsp_memset(wd->wlist.lentry, 0, sizeof(struct gs_wlist_entry_t) * info->num);

    wd->wlist.bsort = gsp_malloc(sizeof(struct gs_wlist_entry_t));
    if (!wd->wlist.bsort)
        goto err_alloc1;

    wd->wlist.maxslen = info->maxslen;
    for (i = 0; i < info->num; i++)
    {
        /* TODO string fields can be of different size */
        wd->wlist.lentry[i].st0 = gsp_malloc(info->maxslen);
        wd->wlist.lentry[i].st1 = gsp_malloc(info->maxslen);
        if (!(wd->wlist.lentry[i].st0 && wd->wlist.lentry[i].st1))
            goto err_alloc2;
        /* make zero-length string */
        *wd->wlist.lentry[i].st0 = 0;
        *wd->wlist.lentry[i].st1 = 0;
    }

    /* custom fields */
    {
        wd->wlist.num     = info->num;
        wd->wlist.font    = info->font;
        wd->wlist.width   = win->width  - info->x;
        wd->wlist.height  = win->height - info->y;
        if (wd->wlist.width > info->width)
            wd->wlist.width = info->width;
        if (wd->wlist.height > info->height)
            wd->wlist.height = info->height;
        wd->wlist.idx     = 0;
        wd->wlist.sel     = -1;
        wd->wlist.cur     = 0;
        wd->wlist.act     = 0;
        wd->wlist.iwidth  = info->iwidth;
        wd->wlist.iheight = info->iheight;
        /* if image does not fit in dimensions - don't use images */
        if ((wd->wlist.iwidth + 2 * WLIST_IMAGE_PAD) > wd->wlist.width)
            wd->wlist.iwidth = 0;
        if ((wd->wlist.iheight + 2 * WLIST_IMAGE_PAD) > wd->wlist.height)
            wd->wlist.iheight = 0;

        if (wd->wlist.iheight)
            wd->wlist.nentr = wd->wlist.height / (wd->wlist.iheight + 2 * WLIST_IMAGE_PAD);
        else
            wd->wlist.nentr = wd->wlist.height / wd->wlist.font->height;
    }

    /* generic fields */
    {
        wd->type        = GS_WIDGET_TYPE_LIST;
        wd->destructor  = gs_wlist_destroy;
        wd->x0          = info->x;
        wd->y0          = info->y;
        wd->x1          = info->x + wd->wlist.width  - 1;
        wd->y1          = info->y + wd->wlist.height - 1;
        wd->touch_cb    = NULL;
    }

    /* allocate memory for images */
    wd->wlist.image = NULL;
    if (wd->wlist.iwidth && wd->wlist.iheight)
    {
        wd->wlist.image = gsp_malloc(sizeof(union gs_pixel_t *) * info->num);
        if (!wd->wlist.image)
            goto err_alloc3;
        gsp_memset(wd->wlist.image, 0, sizeof(char*) * info->num);
        for (i = 0; i < info->num; i++)
        {
            wd->wlist.image[i] =
                gsp_malloc(sizeof (union gs_pixel_t) * wd->wlist.iwidth * wd->wlist.iheight);
            if (!wd->wlist.image[i])
                goto err_alloc4;
            /* load default image if specified */
            if (info->defimage)
                gsp_memcpy(wd->wlist.image[i], info->defimage,
                        (sizeof (union gs_pixel_t) * wd->wlist.iwidth * wd->wlist.iheight));
        }
    }

    gs_win_widget_add(win, wd);
    return wd;
err_alloc4:
    if (!wd->wlist.image)
    {
        if (!wd->wlist.image[i])
            gsp_free(wd->wlist.image[i]);
    }
err_alloc3:
err_alloc2:
    for (i = 0; i < info->num; i++)
    {
        if (!wd->wlist.lentry[i].st0)
            gsp_free(wd->wlist.lentry[i].st0);
        if (!wd->wlist.lentry[i].st1)
            gsp_free(wd->wlist.lentry[i].st1);
    }
    gsp_free(wd->wlist.bsort);
err_alloc1:
    gsp_free(wd->wlist.lentry);
err_alloc0:
    gsp_free(wd);

    return NULL;
}

/*
 *
 */
void gs_wlist_destroy(struct gs_widget_t *wd)
{
    int i;

    if (!wd || wd->type != GS_WIDGET_TYPE_LIST)
        return;

    for (i = 0; i < wd->wlist.num; i++)
    {
        if (wd->wlist.lentry[i].st0)
            gsp_free(wd->wlist.lentry[i].st0);
        if (wd->wlist.lentry[i].st1)
            gsp_free(wd->wlist.lentry[i].st1);
    }
    gsp_free(wd->wlist.bsort);
    gsp_free(wd->wlist.lentry);

    gs_widget_destroy(wd);
}

/*
 * set colors of list's elements
 */
void gs_wlist_set_colors(struct gs_widget_t *wd, int font, int sel, int cur, int cur_edge)
{
    if (!wd || wd->type != GS_WIDGET_TYPE_LIST)
        return;

    wd->wlist.font_color.value    = font;
    wd->wlist.sel_color.value     = sel;
    wd->wlist.cur_color.value     = cur;
    wd->wlist.curedge_color.value = cur_edge;
}

/*
 *
 */
void gs_wlist_set_st0(struct gs_widget_t *wd, int idx, char *st)
{
    int slen;
    int maxstrlen;
    char dot0, dot1, zero;

    if (!wd || wd->type != GS_WIDGET_TYPE_LIST)
        return;

    if (idx >= wd->wlist.num)
        return;

    if (wd->wlist.iwidth && wd->wlist.iheight)
    {
        /* XXX */
        maxstrlen = wd->wlist.maxslen;
//        maxstrlen = (wd->wlist.width - (WLIST_IMAGE_PAD * 2 + wd->wlist.iwidth)) /
//                    wd->wlist.font->width;
//        /* XXX for year carried to separate line */
//        maxstrlen += 7;
    } else {
        maxstrlen = (wd->wlist.width / wd->wlist.font->width);
    }

    slen = strlen(st);
    if (!st || *st == 0 || slen > wd->wlist.maxslen)
        return;

    /* truncate string to displayed length only, make dots */
    dot0 = 0;
    dot1 = 0;
    zero = 0;
    if (slen > 2 && slen > maxstrlen)
    {
        dot0 = st[maxstrlen - 2];
        dot1 = st[maxstrlen - 1];
        zero = st[maxstrlen - 0];

        st[maxstrlen - 2] = '.';
        st[maxstrlen - 1] = '.';
        st[maxstrlen - 0] = 0;
    }

    strcpy(wd->wlist.lentry[idx].st0, st);

    /* place characters back */
    if (dot0)
    {
        st[maxstrlen - 2] = dot0;
        st[maxstrlen - 1] = dot1;
        st[maxstrlen - 0] = zero;
    }
}

/*
 *
 */
void gs_wlist_set_st1(struct gs_widget_t *wd, int idx, char *st)
{
    int slen;

    if (!wd || wd->type != GS_WIDGET_TYPE_LIST)
        return;

    if (idx >= wd->wlist.num)
        return;

    slen = strlen(st);
    if (!st || *st == 0 || slen > wd->wlist.maxslen)
        return;

    strcpy(wd->wlist.lentry[idx].st1, st);
    wd->wlist.lentry[idx].hash1 = pear32((uint8*)st, slen);
}

/*
 * get entry value from list
 *
 * RETURN
 *     NULL on error, string entry otherwise
 */
char* gs_wlist_get_st0(struct gs_widget_t *wd, int idx)
{
    if (!wd || wd->type != GS_WIDGET_TYPE_LIST)
        return NULL;

    if (idx > wd->wlist.act)
        return NULL;

    return wd->wlist.lentry[idx].st0;
}

/*
 * get entry value from list
 *
 * RETURN
 *     NULL on error, string entry otherwise
 */
char* gs_wlist_get_st1(struct gs_widget_t *wd, int idx)
{
    if (!wd || wd->type != GS_WIDGET_TYPE_LIST)
        return NULL;

    if (idx > wd->wlist.act)
        return NULL;

    return wd->wlist.lentry[idx].st1;
}

/*
 * get index of first found entry with matched hash
 *
 * RETURN
 *     -1 on error or index entry on success
 */
int gs_wlist_search_hash1(struct gs_widget_t *wd, uint32 hash)
{
    int i;
    if (!wd || wd->type != GS_WIDGET_TYPE_LIST)
        return -1;

    for (i = 0; i <= wd->wlist.act; i++)
    {
        if (wd->wlist.lentry[i].hash1 == hash)
            return i;
    }
    return -1;
}

/*
 *
 */
void gs_wlist_set_image(struct gs_widget_t *wd, int idx, union gs_pixel_t *image)
{
    if (!wd || wd->type != GS_WIDGET_TYPE_LIST)
        return;

    if (idx >= wd->wlist.num)
        return;

    if (!image || !wd->wlist.iwidth || !wd->wlist.iheight)
        return;

    gsp_memcpy(wd->wlist.image[idx], image,
            (sizeof (union gs_pixel_t) * wd->wlist.iwidth * wd->wlist.iheight));
}

/*
 * set last active index of entry
 */
void gs_wlist_set_active(struct gs_widget_t *wd, int idx)
{
    if (!wd || wd->type != GS_WIDGET_TYPE_LIST)
        return;

    if (idx >= wd->wlist.num)
        return;

    wd->wlist.act = idx;
}

/*
 * set selected index of entry
 */
void gs_wlist_set_sel(struct gs_widget_t *wd, int dir, int sel)
{
    if (!wd || wd->type != GS_WIDGET_TYPE_LIST)
        return;

    if (dir != GS_WLIST_SET_SEL_VALUE
            && wd->wlist.sel < 0)
        return;

    switch (dir)
    {
        case GS_WLIST_SET_SEL_PREV:
            while (sel--)
            {
                if (wd->wlist.sel)
                    wd->wlist.sel--;
                else
                    return;
            }
            break;
        case GS_WLIST_SET_SEL_NEXT:
            while (sel--)
            {
                if (wd->wlist.sel < wd->wlist.act)
                    wd->wlist.sel++;
                else
                    return;

                /* NOTE for autoscroll of playback */
                if (wd->wlist.sel > 0)
                {
                    while (wd->wlist.sel < wd->wlist.idx)
                        gs_wlist_scroll(wd, GS_WLIST_SCROLL_UP);

                    while (wd->wlist.sel >= (wd->wlist.idx + wd->wlist.nentr))
                        gs_wlist_scroll(wd, GS_WLIST_SCROLL_DOWN);
                }
            }
            break;
        case GS_WLIST_SET_SEL_VALUE:
            if (sel > wd->wlist.act)
                return;
            wd->wlist.sel = sel;

            /* NOTE for autoscroll of playback */
            if (wd->wlist.sel >= 0)
            {
                while (wd->wlist.sel < wd->wlist.idx)
                    gs_wlist_scroll(wd, GS_WLIST_SCROLL_UP);

                while (wd->wlist.sel >= (wd->wlist.idx + wd->wlist.nentr))
                    gs_wlist_scroll(wd, GS_WLIST_SCROLL_DOWN);
            }
            break;
    }
}

/*
 * scroll list up, down
 */
void gs_wlist_scroll(struct gs_widget_t *wd, int dir)
{
    if (!wd || wd->type != GS_WIDGET_TYPE_LIST)
        return;

    switch (dir)
    {
        case GS_WLIST_SCROLL_UP:
            if (wd->wlist.idx > 0)
                wd->wlist.idx--;
            break;
        case GS_WLIST_SCROLL_DOWN:
            if ((wd->wlist.act - wd->wlist.idx) >= wd->wlist.nentr
                    && wd->wlist.idx < wd->wlist.act)
                wd->wlist.idx++;
            break;
    }
}

/*
 * move cursor up/down and scroll list if necessary
 */
void gs_wlist_set_cur(struct gs_widget_t *wd, int dir, int cur)
{
    if (!wd || wd->type != GS_WIDGET_TYPE_LIST)
        return;

    if (dir != GS_WLIST_SET_CUR_VALUE
            && wd->wlist.cur < 0)
        return;

    switch (dir)
    {
        case GS_WLIST_SET_CUR_PREV:
            while (cur--)
            {
                if (wd->wlist.cur)
                    wd->wlist.cur--;
                else
                    return;

                if (wd->wlist.cur >= (wd->wlist.idx + wd->wlist.nentr) && wd->wlist.nentr)
                    wd->wlist.cur = wd->wlist.idx + wd->wlist.nentr - 1;

                while (wd->wlist.cur < wd->wlist.idx)
                    gs_wlist_scroll(wd, GS_WLIST_SCROLL_UP);
            }
            break;
        case GS_WLIST_SET_CUR_NEXT:
            while (cur--)
            {
                if (wd->wlist.cur < wd->wlist.act)
                    wd->wlist.cur++;
                else
                    return;

                if (wd->wlist.cur < wd->wlist.idx)
                    wd->wlist.cur = wd->wlist.idx;

                while (wd->wlist.cur >= (wd->wlist.idx + wd->wlist.nentr))
                    gs_wlist_scroll(wd, GS_WLIST_SCROLL_DOWN);
            }
            break;
        case GS_WLIST_SET_CUR_VALUE:
            /* set cursor to specific position and center if possible */
            if (cur > wd->wlist.act)
                return;
            wd->wlist.cur = cur;

            if (wd->wlist.cur > (wd->wlist.nentr / 2))
            {
                wd->wlist.idx = wd->wlist.cur - (wd->wlist.nentr / 2);
                if (wd->wlist.idx < 0)
                    wd->wlist.idx = 0;
            } else {
                wd->wlist.idx = 0;
            }

            break;
    }
}

/*
 * draw entries of list to it's parent window
 */
static char trunk_string(char *st, int maxlen);

void gs_wlist_draw(struct gs_widget_t *wd)
{
    struct gs_win_t *win;
    int y;
    int i, n;
    char *st;
    int maxstrlen;
    int useimage;

    if (!wd || !wd->pwin || wd->type != GS_WIDGET_TYPE_LIST)
        return;
    win = wd->pwin;

    useimage = 0;

    /* fill window with default background */
    gs_prim_fill_rect(win, wd->x0, wd->y0, wd->x1, wd->y1, win->bgcolor.value);

    if (wd->wlist.iwidth && wd->wlist.iheight)
    {
        useimage = 1;
        maxstrlen = (wd->wlist.width - (WLIST_IMAGE_PAD * 2 + wd->wlist.iwidth)) /
                    wd->wlist.font->width;

    } else {
        maxstrlen = (wd->wlist.width / wd->wlist.font->width);
    }

    y = wd->y0;
    for (n = 0, i = wd->wlist.idx; i <= wd->wlist.act; i++, n++)
    {
        /* NOTE */
        if (n >= wd->wlist.nentr)
            break;

        /* sanity check */ 
        if (i < 0)
            break;

        st  = wd->wlist.lentry[i].st0;

        /* draw selection, image, text and cursor */
        {
            int x0, y0, x1, y1;
            int bgcolor;

            x0 = wd->x0;
            y0 = y;
            x1 = x0 + wd->wlist.width - 1;

            if (useimage)
                y1 = y0 + wd->wlist.iheight + 2 * WLIST_IMAGE_PAD - 1;
            else
                y1 = y0 + wd->wlist.font->height - 1;
            if (i == wd->wlist.sel)
            {
                bgcolor = wd->wlist.sel_color.value;
                gs_prim_fill_rect(win, x0, y0, x1, y1, bgcolor);
            } else {
                bgcolor = win->bgcolor.value;
            }
            
            if (useimage)
            {
                gs_image_map(win,
                        wd->x0 + WLIST_IMAGE_PAD, y + WLIST_IMAGE_PAD,
                        (uint8*)wd->wlist.image[i], wd->wlist.iwidth, wd->wlist.iheight);

                /* TODO unify, now only for album list */
                {
                    /* 1987 - Album Name */
#define YEAR_STRLEN    4
//                    char ch;
                    int ofsx, ofsy;
                    char year[YEAR_STRLEN + 1];
                    char *album;

                    /* XXX no check for out of bounds (height especially) */
                    if (strlen(st) > (YEAR_STRLEN + 3))
                    {
                        strncpy(year, st, YEAR_STRLEN);
                        year[YEAR_STRLEN] = 0;

                        ofsx  = (WLIST_IMAGE_PAD * 2 + wd->wlist.iwidth);
                        ofsx += (wd->wlist.width - (WLIST_IMAGE_PAD * 2 + wd->wlist.iwidth)) / 2;

                        ofsy = (wd->wlist.iheight + (WLIST_IMAGE_PAD * 2)) / 4;

                        gs_text_put_centered(win, year, wd->x0 + ofsx, y + ofsy,
                                wd->wlist.font_color.value, bgcolor, wd->wlist.font);

                        album = &st[YEAR_STRLEN + 3];
                        {
                            int k, n, m;
                            char *alb, *st;

                            alb = album;
                            for (k = 0; k < 2; k++)
                            {
                                n = 0;
                                m = 0;
                                st = alb;
                                while (n <= maxstrlen) {
                                    if (*st == 0)
                                        break;
                                    if (*st == ' ')
                                        m = n;
                                    st++;
                                    n++;
                                }

                                if (!n)
                                    break;

                                if (m && n > maxstrlen)
                                    n = m + 1;

//                                dprint("<alb:>s*,4dn", alb, n);

                                ofsy = (wd->wlist.iheight + (WLIST_IMAGE_PAD * 2)) * (2 + k) / 4;
                                gs_text_nput_centered(win, alb, n, wd->x0 + ofsx, y + ofsy,
                                        wd->wlist.font_color.value, bgcolor, wd->wlist.font);

                                alb += n;
                            }
                        }
//                        {
//                            ofsy = (wd->wlist.iheight + (WLIST_IMAGE_PAD * 2)) * 2 / 4;
//
//                            ch = trunk_string(album, maxstrlen);
//                            gs_text_put_centered(win, album, wd->x0 + ofsx, y + ofsy,
//                                    wd->wlist.font_color.value, bgcolor, wd->wlist.font);
//                            if (ch)
//                            {
//                                album[maxstrlen] = ch;
//
//                                album += maxstrlen;
//
//                                ofsy = (wd->wlist.iheight + (WLIST_IMAGE_PAD * 2)) * 3 / 4;
//
//                                ch = trunk_string(album, maxstrlen);
//                                gs_text_put_centered(win, album, wd->x0 + ofsx, y + ofsy,
//                                        wd->wlist.font_color.value, bgcolor, wd->wlist.font);
//                                if (ch)
//                                    album[maxstrlen] = ch;
//                            }
//                        }
                    }
                }
            } else {
                char ch;

                ch = trunk_string(st, maxstrlen);
                gs_text_put(win, st, wd->x0, y,
                        wd->wlist.font_color.value, bgcolor, wd->wlist.font);
                if (ch)
                    st[maxstrlen] = ch;
            }

            if (i == wd->wlist.cur)
            {
                if (i == 0 || i == wd->wlist.act)
                    gs_prim_draw_rect(win, x0, y0, x1, y1, 2, wd->wlist.curedge_color.value);
                else
                    gs_prim_draw_rect(win, x0, y0, x1, y1, 2, wd->wlist.cur_color.value);
            }
        }

        if (useimage)
            y += wd->wlist.iheight + WLIST_IMAGE_PAD * 2;
        else
            y += wd->wlist.font->height;
    }
}

/*
 * truncate string to specific length
 *
 * RETURN
 *     character that was replaced with zero or zero
 *     if string length less then "len" argument
 */
static char trunk_string(char *st, int maxlen)
{
    int slen;
    char ch;

    ch = 0;
    slen = strlen(st);
    if (slen > maxlen)
    {
        ch = st[maxlen];
        st[maxlen] = 0;
    }

    return ch;
}

/*
 * sort entries in list by name
 *
 * NOTE entries with length greater then buffer size not sorted
 */
void gs_wlist_sort(struct gs_widget_t *wd)
{
    int n, m, swap;
//#define SORT_BUF_SIZE    255
//    char sortbuf[SORT_BUF_SIZE + 1]; /* NOTE +1 for null-terminator */

    if (!wd || wd->type != GS_WIDGET_TYPE_LIST)
        return;

    for (n = 1; n < (wd->wlist.act + 1) - 1; n++)
    {
        swap = 0;
        for (m = 0; m < ((wd->wlist.act + 1) - n); m++)
        {
            if (strcmp(wd->wlist.lentry[m].st0, wd->wlist.lentry[m + 1].st0) > 0)
            {
                /* NOTE string entries not copied, pointer to strings swaps instead */
                memcpy(wd->wlist.bsort        ,  &wd->wlist.lentry[m    ], sizeof(struct gs_wlist_entry_t));
                memcpy(&wd->wlist.lentry[m    ], &wd->wlist.lentry[m + 1], sizeof(struct gs_wlist_entry_t));
                memcpy(&wd->wlist.lentry[m + 1], wd->wlist.bsort         , sizeof(struct gs_wlist_entry_t));
                swap = 1;

//                if (strlen(wd->wlist.entry[m]) > SORT_BUF_SIZE)
//                    continue;
//
//                strcpy(sortbuf               , wd->wlist.entry[m    ]);
//                strcpy(wd->wlist.entry[m    ], wd->wlist.entry[m + 1]);
//                strcpy(wd->wlist.entry[m + 1], sortbuf               );
//                swap = 1;
            }
        }
        if (!swap)
            break;
    }
}

