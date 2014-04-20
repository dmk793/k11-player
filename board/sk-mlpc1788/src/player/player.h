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

#ifndef PLAYER_H
#define PLAYER_H

#include <types.h>
#include <os.h>
#include "../fat_io_lib/fat_filelib.h"


#define PLAYER_MUSIC_DIR     "/music"
#define PLAYER_PATH_MAXLEN   4096

void player_task();

struct player_msg_t {
#define PLAYER_MSG_ID_BUTTON         0x00
#define PLAYER_MSG_ID_DEC_POSITION   0x02
#define PLAYER_MSG_ID_DEC_NEXTTRACK  0x03
#define PLAYER_MSG_ID_DEC_STOPPED    0x04
#define PLAYER_MSG_ID_DEC_PAUSED     0x05
#define PLAYER_MSG_ID_DEC_PLAY       0x06
#define PLAYER_MSG_ID_DEC_VOLUME     0x07
    int id;
    union {
        struct pmsg_empty_t {
        } empty;
        struct pmsg_button_t {
            uint32 code;
        } button;
        struct pmsg_volume_t {
            int value;
        } volume;
        struct pmsg_position_t {
            int value;
            int time;
        } position;
    };
};

struct decoder_msg_t {
#define DECODER_MSG_ID_PLAY       0x00
#define DECODER_MSG_ID_STOP       0x01
#define DECODER_MSG_ID_PAUSE      0x02
#define DECODER_MSG_ID_FAST_PLAY  0x03
#define DECODER_MSG_ID_INTR_PLAY  0x04
#define DECODER_MSG_ID_VOLUME     0x05
    int id;
    union {
        struct dmsg_empty_t {
        } empty;
        struct dmsg_play_t {
            uint32 flen;
        } play;
        struct dmsg_volume_t {
            int value;
        } volume;
    };
};

/*
 * NOTE
 * Messages sended to player in non-blocking mode to prevent
 * interlock of player and decoder.
 */

#define PLAYER_MSG_SIZE(m) (sizeof(int) + sizeof(struct m))
#define PLAYER_SEND_MSG(pm, size) \
            os_queue_add(player.qplayer, OS_FLAG_NOWAIT, OS_WAIT_FOREVER, pm, PLAYER_MSG_SIZE(size));
#define PLAYER_SEND_MSG_BLOCKING(pm, size) \
            os_queue_add(player.qplayer, OS_FLAG_NONE, OS_WAIT_FOREVER, pm, PLAYER_MSG_SIZE(size));
#define PLAYER_QUEUE_FLUSH(player) \
            os_queue_flush(player.qplayer)

#define DECODER_MSG_SIZE(m) (sizeof(int) + sizeof(struct m))
#define DECODER_SEND_MSG(pm, size) \
            os_queue_add(player.qdecoder, OS_FLAG_NONE, OS_WAIT_FOREVER, pm, DECODER_MSG_SIZE(size));
#define DECODER_QUEUE_FLUSH(player) \
            os_queue_flush(player.qdecoder)

struct player_t {

#define PLAYER_EVENT_INIT     (1 << 0)
    BASE_TYPE event;
    struct gs_win_t *mwin;
    uint8 *filebuf;

#define PLAYER_MODE_NOP               0
#define PLAYER_MODE_ARTIST_BROWSER    1
#define PLAYER_MODE_ALBUM_BROWSER     2
#define PLAYER_MODE_TRACK_BROWSER     3
    int mode;     /* player mode */

    int volume;   /* volume level */

#define PLAYER_REPEAT_MODE_NONE     0
#define PLAYER_REPEAT_MODE_TRACK    1
#define PLAYER_REPEAT_MODE_ALBUM    2
#define PLAYER_REPEAT_MODE_LAST     PLAYER_REPEAT_MODE_ALBUM
    int repeat;   /* repeat mode */

    /* miscellaneous images */
    union gs_pixel_t *nocover_small;
    union gs_pixel_t *nocover_big;
    union gs_pixel_t *simage;
    union gs_pixel_t *bimage;
    union gs_pixel_t *play_ico;
    union gs_pixel_t *pause_ico;
    union gs_pixel_t *stop_ico;
    union gs_pixel_t *repeat_none_ico;
    union gs_pixel_t *repeat_track_ico;
    union gs_pixel_t *repeat_album_ico;

    /* widgets */
    struct {
        struct gs_widget_t *artlist;     /* list that contain artists directories */
        struct gs_widget_t *alblist;     /* list that contain albums directories */
        struct gs_widget_t *trcklist;    /* list that contain tracks of album */
        struct gs_widget_t *pbar;        /* progress bar for played track */
        struct gs_widget_t *time;        /* decode time */
        struct gs_widget_t *header;      /* header for artist and album name */
        struct gs_widget_t *ppause;      /* play, pause, stop pixmap */
        struct gs_widget_t *repeat;      /* repeat pixmap */
        struct gs_widget_t *volume;      /* volume */
    } widget;

    char *path;   /* temporary path location */

    /* player and decoder event queues */
    struct os_queue_t *qplayer;
    struct os_queue_t *qdecoder;
};

#define FILE_CACHE_ENTRY_SIZE    2048
#define FILE_CACHE_ENTRIES       64
struct file_cache_t {
    FL_FILE *file;           /* file handle */
#define FILE_CACHE_MUTEX       (1 << 0)
    BASE_TYPE mutex;

#define FILE_CACHE_FILLED_EVENT (1 << 0)
    BASE_TYPE event;

    struct fcache_entry_t {
        uint8 buf[FILE_CACHE_ENTRY_SIZE];
        int len;
    } entry[FILE_CACHE_ENTRIES];
    int wp;
    int cnt;
    int drained;
};

extern struct player_t player;

#define PLAYER_INFOMSG_NO_MEDIA         "   No media   "
#define PLAYER_INFOMSG_PLEASE_WAIT      "Please wait..."

void player_large_msg(char *st);
int player_scan_artists();
int player_scan_albums(char *artist);
int player_scan_tracks(char *artist, char *album);
int player_scan_thumb(char *artist);
int player_get_cover(char *artist, char* album);
char *player_mkpath(char **args);

uint32 player_fcache_open(char *path);
void player_fcache_close();
void player_fcache_fill();
void player_fcache_remove();
struct fcache_entry_t * player_fcache_get(int *drained);

#endif

