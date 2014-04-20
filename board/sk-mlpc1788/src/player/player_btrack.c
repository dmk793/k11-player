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

/*
 * Artist browser.
 *
 * TODO
 *     * refresh only part of window on simple events like progress redraw
 */
#include <types.h>
#include <debug.h>
#include <string.h>
#include <util.h>
#include "player_btrack.h"
#include "player_layout.h"
#include "../buttons.h"
#include "../gs/gs.h"
#include "../image/image.h"

#define DEBUG_BTRACK

#ifdef DEBUG_BTRACK
    #define DPRINT(fmt, ...) dprint(fmt, __VA_ARGS__)
#else
    #define DPRINT(fmt, ...)
#endif

static char *player_btrack_gettrack(int next);

#define BTRACK_PLAY_CURRENT    0
#define BTRACK_PLAY_NEXT       1
static void player_btrack_playfile(int next);

static struct {
    char *artist;
    char *album;

#define PLAYER_CMD_PLAY   (1 << 0)
#define PLAYER_CMD_BACK   (1 << 1)
    uint32 cmd;
} btrack;

#define TRANSPARENT_COLOR    0x00ff00ff

/*
 * refresh widgets on entry
 */
void player_btrack_onenter()
{
    union gs_pixel_t avcolor;

    /* store artist, album information */
    btrack.artist = GS_WLIST_GET_CUR_ST1(player.widget.artlist);
    btrack.album  = GS_WLIST_GET_CUR_ST1(player.widget.alblist);

    /* nothing to yet yet */
    btrack.cmd = 0;

    /* get average color of cover, change colors of widgets */
#define SCAN_BORDER_WIDTH    4
    avcolor.value = image_get_avcolor((uint8*)player.bimage, IMAGE_BIG_WIDTH, IMAGE_BIG_HEIGHT, SCAN_BORDER_WIDTH);
    {
        union gs_pixel_t bgcolor, font, sel, cur, cur_edge;
        struct yuv_color_t yuv;

        /* background color */
        {
            bgcolor.value = avcolor.value;

            gs_util_rgb2yuv(&bgcolor, &yuv);
#define MAX_Y  40
            if (yuv.y > MAX_Y)
            {
                yuv.y = MAX_Y;
                gs_util_yuv2rgb(&yuv, &bgcolor);
            }
        }
        player.mwin->bgcolor.value = bgcolor.value;

        /* selection color */
        {
            sel.value = bgcolor.value;
            gs_util_rgb2yuv(&sel, &yuv);
#define SEL_MIN_Y      40
#define SEL_ADJUST_Y   30
            if (yuv.y < SEL_MIN_Y)
                yuv.y += SEL_ADJUST_Y;
            else
                yuv.y -= SEL_ADJUST_Y;

            gs_util_yuv2rgb(&yuv, &sel);
        }

        /* font color */
        font.value = (bgcolor.value ^ 0xffffff) & (~0x00404040);

        /* cursor color */
        {
            cur.value      = CUR_COLOR;
            cur_edge.value = CUR_EDGE_COLOR;
        }

        gs_wlist_set_colors(player.widget.trcklist,
                font.value, sel.value, cur.value, cur_edge.value);
        gs_wpbar_set_colors(player.widget.pbar, sel.value, font.value, font.value ^ 0x00ffffff);
        gs_wvolume_set_colors(player.widget.volume, sel.value, sel.value ^ 0x00ffffff, font.value ^ 0x00ffffff);
        gs_wtext_set_colors(player.widget.header, font.value, bgcolor.value);
        gs_wtext_set_colors(player.widget.time,   font.value, bgcolor.value);

        /* XXX substitute pixeles in play pause stop, repeat icons to desired color */
        {
            int len;
            union gs_pixel_t *play, *pause, *stop;
            union gs_pixel_t *repeat0, *repeat1, *repeat;

            len   = BROWSER_ICO_WIDTH * BROWSER_ICO_HEIGHT;
            play  = player.play_ico;
            pause = player.pause_ico;
            stop  = player.stop_ico;
            repeat0 = player.repeat_none_ico;
            repeat1 = player.repeat_track_ico;
            repeat = player.repeat_album_ico;
            while (len--)
            {
                if ((play->value & 0x00ffffff) != TRANSPARENT_COLOR)
                    play->value = font.value;
                if ((pause->value & 0x00ffffff) != TRANSPARENT_COLOR)
                    pause->value = font.value;
                if ((stop->value & 0x00ffffff) != TRANSPARENT_COLOR)
                    stop->value = font.value;
                if ((repeat0->value & 0x00ffffff) != TRANSPARENT_COLOR)
                    repeat0->value = font.value;
                if ((repeat1->value & 0x00ffffff) != TRANSPARENT_COLOR)
                    repeat1->value = font.value;
                if ((repeat->value & 0x00ffffff) != TRANSPARENT_COLOR)
                    repeat->value = font.value;

                play++;
                pause++;
                stop++;
                repeat0++;
                repeat1++;
                repeat++;
            }
        }
    }

    gs_win_clear(player.mwin);

    /* XXX */
    {
        sprint(player.path, "s_*/_s",
                GS_WLIST_GET_CUR_ST0(player.widget.artlist),
                GS_WLIST_GET_CUR_ST0(player.widget.alblist));

        gs_wtext_set_text(player.widget.header, player.path);
        gs_wtext_draw(player.widget.header, GS_WTEXT_DRAW_CENTERED);
    }

    {
        gs_wtext_set_text(player.widget.time, "--:--:--");
        gs_wtext_draw(player.widget.time, GS_WTEXT_DRAW_CENTERED);
    }

//    /* XXX */
//    gs_wvolume_set_value(player.widget.volume, 30);

    gs_wpbar_set_value(player.widget.pbar, 0);
    gs_wpbar_draw(player.widget.pbar);
    gs_wvolume_draw(player.widget.volume);

    gs_wlist_draw(player.widget.trcklist);
    gs_image_map(player.mwin, IMAGE_BIG_PAD, (MAIN_WINDOW_HEIGHT - IMAGE_BIG_WIDTH) / 2,
            (uint8*)player.bimage, IMAGE_BIG_WIDTH, IMAGE_BIG_HEIGHT);

    /* draw stop ico */
    gs_wpixmap_set_image(player.widget.ppause, player.stop_ico);
    gs_wpixmap_draw2(player.widget.ppause, TRANSPARENT_COLOR);

    /* draw repeat ico */
    switch (player.repeat)
    {
        case PLAYER_REPEAT_MODE_NONE:  gs_wpixmap_set_image(player.widget.repeat, player.repeat_none_ico); break;
        case PLAYER_REPEAT_MODE_TRACK: gs_wpixmap_set_image(player.widget.repeat, player.repeat_track_ico); break;
        case PLAYER_REPEAT_MODE_ALBUM: gs_wpixmap_set_image(player.widget.repeat, player.repeat_album_ico); break;
    }
    gs_wpixmap_draw2(player.widget.repeat, TRANSPARENT_COLOR);

    gs_win_refresh(player.mwin);
    /* flush queue (remove odd keypress by user) */
    PLAYER_QUEUE_FLUSH(player);
}

/*
 * process message
 */
int player_btrack_procmsg(struct player_msg_t *msg)
{
    struct decoder_msg_t dmsg;

#define BTRACK_BUTTON_UP          BUTTON_UP
#define BTRACK_BUTTON_DOWN        BUTTON_DOWN
#define BTRACK_BUTTON_BACK        BUTTON_BACK
#define BTRACK_BUTTON_REPEAT      BUTTON_PGUP
#define BTRACK_BUTTON_PAUSE       BUTTON_PGDOWN
#define BTRACK_BUTTON_PLAY        BUTTON_ENTER

    uint32 button;
    switch (msg->id)
    {
        case PLAYER_MSG_ID_BUTTON:
            {
                button = msg->button.code;
                if (BUTTON_PRESSED(button, BTRACK_BUTTON_PLAY)) {
                    /* befor playback can be started we should stop decoder */
                    {
                        dmsg.id = DECODER_MSG_ID_STOP;
                        DECODER_SEND_MSG(&dmsg, dmsg_empty_t);
                    }
                    BITMASK_SET(btrack.cmd, PLAYER_CMD_PLAY);
                } else if (BUTTON_PRESSED(button, BTRACK_BUTTON_PAUSE)) {
                    /* pause decoder */
                    {
                        dmsg.id = DECODER_MSG_ID_PAUSE;
                        DECODER_SEND_MSG(&dmsg, dmsg_empty_t);
                    }
                } else if (BUTTON_PRESSED(button, BTRACK_BUTTON_BACK)) {
                    /* stop decoder */
                    {
                        dmsg.id = DECODER_MSG_ID_STOP;
                        DECODER_SEND_MSG(&dmsg, dmsg_empty_t);
                    }
                    BITMASK_SET(btrack.cmd, PLAYER_CMD_BACK);
                } else if (BUTTON_PRESSED(button, BTRACK_BUTTON_DOWN)) {
                    gs_wlist_set_cur(player.widget.trcklist, GS_WLIST_SET_CUR_NEXT, 1);
                    gs_wlist_draw(player.widget.trcklist);
                    gs_widget_refresh(player.widget.trcklist);
                    gs_win_refresh_widgets(player.mwin);
                } else if (BUTTON_PRESSED(button, BTRACK_BUTTON_UP)) {
                    gs_wlist_set_cur(player.widget.trcklist, GS_WLIST_SET_CUR_PREV, 1);
                    gs_wlist_draw(player.widget.trcklist);
                    gs_widget_refresh(player.widget.trcklist);
                    gs_win_refresh_widgets(player.mwin);
                } else if (BUTTON_PRESSED(button, BTRACK_BUTTON_REPEAT)) {
                    player.repeat++;
                    if (player.repeat > PLAYER_REPEAT_MODE_LAST)
                        player.repeat = PLAYER_REPEAT_MODE_NONE;
                    switch (player.repeat)
                    {
                        case PLAYER_REPEAT_MODE_NONE:  gs_wpixmap_set_image(player.widget.repeat, player.repeat_none_ico); break;
                        case PLAYER_REPEAT_MODE_TRACK: gs_wpixmap_set_image(player.widget.repeat, player.repeat_track_ico); break;
                        case PLAYER_REPEAT_MODE_ALBUM: gs_wpixmap_set_image(player.widget.repeat, player.repeat_album_ico); break;
                    }
                    gs_wpixmap_draw2(player.widget.repeat, TRANSPARENT_COLOR);
                    gs_widget_refresh(player.widget.repeat);
                    gs_win_refresh_widgets(player.mwin);
                } else if (BUTTON_PRESSED(button, ENCODER_PLUS)) {
//                    dprint("sn", "encoder+");

#define VOLUME_INCREMENT    5
                    dmsg.id           = DECODER_MSG_ID_VOLUME;
                    dmsg.volume.value = (player.volume + VOLUME_INCREMENT) < 100 ?
                        (player.volume + VOLUME_INCREMENT) : 100;
                    DECODER_SEND_MSG(&dmsg, dmsg_volume_t);
                } else if (BUTTON_PRESSED(button, ENCODER_MINUS)) {
//                    dprint("sn", "encoder-");

                    dmsg.id           = DECODER_MSG_ID_VOLUME;
                    dmsg.volume.value = (player.volume - VOLUME_INCREMENT) > 0 ?
                        (player.volume - VOLUME_INCREMENT) : 0;
                    DECODER_SEND_MSG(&dmsg, dmsg_volume_t);
                }
            }
            break;
        case PLAYER_MSG_ID_DEC_VOLUME:
            {
                player.volume = msg->volume.value;

                gs_wvolume_set_value(player.widget.volume, msg->volume.value);
                gs_wvolume_draw(player.widget.volume);
                gs_widget_refresh(player.widget.volume);
                gs_win_refresh_widgets(player.mwin);
            }
            break;
        case PLAYER_MSG_ID_DEC_NEXTTRACK:
            {
                player_btrack_playfile(BTRACK_PLAY_NEXT);

                gs_wlist_draw(player.widget.trcklist);
                gs_widget_refresh(player.widget.trcklist);
                gs_win_refresh_widgets(player.mwin);
            }
            break;
        case PLAYER_MSG_ID_DEC_POSITION:
            {
                int h, m, s;
                char time[16];

                if (msg->position.time)
                {
                    h =  msg->position.time / 3600;
                    m = (msg->position.time % 3600) / 60;
                    s =  msg->position.time % 60;

                    sprint(time, "s1d*:s1d*:s1d",
                            h > 9 ? "" : "0", h,
                            m > 9 ? "" : "0", m,
                            s > 9 ? "" : "0", s);

                    gs_wtext_set_text(player.widget.time, time);
                } else {
                    gs_wtext_set_text(player.widget.time, "--:--:--");
                }

                gs_wtext_draw(player.widget.time, GS_WTEXT_DRAW_CENTERED);
            }
            gs_wpbar_set_value(player.widget.pbar, msg->position.value);
            gs_wpbar_draw(player.widget.pbar);

            gs_widget_refresh(player.widget.time);
            gs_widget_refresh(player.widget.pbar);

            gs_win_refresh_widgets(player.mwin);
            break;
        case PLAYER_MSG_ID_DEC_STOPPED:
            {
                int refresh;

                if (btrack.cmd & PLAYER_CMD_BACK)
                {
                    BITMASK_CLEAR(btrack.cmd, PLAYER_CMD_BACK);
                    return PLAYER_MODE_ALBUM_BROWSER;
                }

                refresh = 0;
                if (btrack.cmd & PLAYER_CMD_PLAY)
                {
                    BITMASK_CLEAR(btrack.cmd, PLAYER_CMD_PLAY);

                    player_btrack_playfile(BTRACK_PLAY_CURRENT);

                    gs_wlist_draw(player.widget.trcklist);
                    gs_widget_refresh(player.widget.trcklist);
                    refresh = 1;
                }
                if (refresh)
                    gs_win_refresh_widgets(player.mwin);

            }
            break;
        case PLAYER_MSG_ID_DEC_PAUSED:
            {
                /* change icon to paused */
                gs_wpixmap_set_image(player.widget.ppause, player.pause_ico);
                gs_wpixmap_draw2(player.widget.ppause, TRANSPARENT_COLOR);
                gs_widget_refresh(player.widget.ppause);
                gs_win_refresh_widgets(player.mwin);
            }
            break;
        case PLAYER_MSG_ID_DEC_PLAY:
            {
                /* change icon to play */
                gs_wpixmap_set_image(player.widget.ppause, player.play_ico);
                gs_wpixmap_draw2(player.widget.ppause, TRANSPARENT_COLOR);
                gs_widget_refresh(player.widget.ppause);
                gs_win_refresh_widgets(player.mwin);
            }
            break;
    }
    return player.mode;
}

/*
 * get track under cursor/next to selected
 */
static char *player_btrack_gettrack(int next)
{
    char *token[5];
    char *path;
    char *track;

    if (next)
    {
        if ((player.widget.trcklist->wlist.sel >= 0) &&
            (player.widget.trcklist->wlist.sel < player.widget.trcklist->wlist.act))
        {
            if (player.repeat != PLAYER_REPEAT_MODE_TRACK)
                gs_wlist_set_sel(player.widget.trcklist, GS_WLIST_SET_SEL_NEXT, 1);
        } else {
            if (player.repeat == PLAYER_REPEAT_MODE_ALBUM)
                gs_wlist_set_sel(player.widget.trcklist, GS_WLIST_SET_SEL_VALUE, 0);
            else
                return NULL;
        }
    } else {
        /* XXX select under cursor */
        gs_wlist_set_sel(player.widget.trcklist, GS_WLIST_SET_SEL_VALUE, player.widget.trcklist->wlist.cur);
    }

    token[0] = PLAYER_MUSIC_DIR;
    token[1] = btrack.artist;
    token[2] = btrack.album;
    track = GS_WLIST_GET_SEL_ST1(player.widget.trcklist);
    if (!track)
        return NULL;
    token[3] = track;
    token[4] = NULL;

    path = player_mkpath(token);
    if (!path)
    {
        DEBUG_WMSG("can not make path");
        return NULL;
    }

    return path;
}

/*
 *
 */
static void player_btrack_playfile(int next)
{
    struct decoder_msg_t dmsg;
    uint32 flen;
    char *path;

    player_fcache_close();

    /* start new song */
    path = player_btrack_gettrack(next);
    if (path)
    {
        flen = player_fcache_open(path);
        if (flen)
        {
            dmsg.id = DECODER_MSG_ID_PLAY;
            dmsg.play.flen = flen;
            DECODER_SEND_MSG(&dmsg, dmsg_play_t);
        }
    } else {
        gs_wlist_set_sel(player.widget.trcklist, GS_WLIST_SET_SEL_VALUE, -1);
        DEBUG_IMSG("playlist end/error");
    }
}

