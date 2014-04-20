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
 *
 */
#include <debug.h>
#include <os.h>
#include <pearson.h>
#include <stimer.h>
#include "string.h"
#include "player.h"
#include "player_bartist.h"
#include "player_balbum.h"
#include "player_btrack.h"
#include "player_layout.h"
#include "../gs/gs.h"
#include "../sdcard/sdcard.h"
#include "../image/image.h"
#include "../fat_io_lib/fat_filelib.h"
#include "../buttons.h"
#include "../vs1053b/vs1053b.h"
#include "../vs1053b/decoder.h"
#include "../nvram.h"

#define DEBUG_PLAYER

#ifdef DEBUG_PLAYER
    #define DPRINT(fmt, ...)  dprint(fmt, __VA_ARGS__)
#else
    #define DPRINT(fmt, ...)
#endif

/********************************************
 * Miscellaneous settings
 */
#define DIR_CHAR  '/'

#define LARGE_MSG_FONT       GS_FONT_9X16

#define FILEBUF_SIZE         (1 * 1024 * 1024)
#define ARTIST_NAME_MAXLEN   64
#define ARTIST_ENTRIES_MAX   4096
#define ALBUM_NAME_MAXLEN    128
#define ALBUM_ENTRIES_MAX    64
/* 
 * Demilich - Nesphite, track 9 - 131 symbol :)
 * "The Planet That Once Used to Absorb Flesh in Order to Achieve Divinity and Immortality (Suffocated to the Flesh That It Desired...)"
 */
#define TRACK_NAME_MAXLEN    256
#define TRACK_ENTRIES_MAX    256 /* Agoraphobic Nosebleed - Altered States of America. 100 tracks */

#define PLAYER_QUEUE_LENGTH    4
#define DECODER_QUEUE_LENGTH   4

/*********************************************/

/* XXX for recursive locking functionality */
extern volatile struct os_taskcb_t *os_current_taskcb;          /* pointer to current task's control block */

struct fs_mutex_t {
#define FS_MUTEX_0  (1 << 0)
#define FS_MUTEX_1  (1 << 1)
    BASE_TYPE mutex;
    volatile struct os_taskcb_t *owner;
    int cnt;
};

struct player_t player;
//static struct file_cache_t fcache __attribute__((section("sram")));
static struct file_cache_t fcache;
static struct fs_mutex_t fsmutex;

static void player_init();
#define MEDIA_INSERT    0
#define MEDIA_REMOVE    1
void player_large_msg(char *st);
static int player_wait_media(int state);
static void player_browser();
static int player_load_sys();
static int player_read_full_file(char *path);

static void player_fs_lock(void);
static void player_fs_unlock(void);

/*
 *
 */
void player_task()
{
    memset(&fcache, 0, sizeof(struct file_cache_t));

    player_init();

    if (!vs1053b_init())
    {
        player_large_msg("Decoder init error");
        os_wait(OS_WAIT_FOREVER);
    }

    os_event_raise(&player.event, PLAYER_EVENT_INIT);

    /* XXX */
    player.volume = 50;
    gs_wvolume_set_value(player.widget.volume, decoder_set_volume(player.volume));

    while (1)
    {
        player_large_msg(PLAYER_INFOMSG_NO_MEDIA);
        if (player_wait_media(MEDIA_INSERT))
        {
            player_large_msg(PLAYER_INFOMSG_PLEASE_WAIT);
            player_browser();
        } else {
            player_wait_media(MEDIA_REMOVE);
        }
    }
}

/*
 *
 */
static void player_init()
{
    struct gs_win_t *win;
    struct gs_wlist_init_info_t linfo;

    nvram_load();
    sdcard_hw_init();

    while (!gs_initialized())
        os_wait_ms(10);

    /*
     * NOTE
     * Should fall to trap if allocation failed, don't check.
     * Also no neccessary to allocate this stuff in dynamic memory.
     */
    player.filebuf          = os_malloc(FILEBUF_SIZE);
    player.path             = os_malloc(PLAYER_PATH_MAXLEN + 1 /* +1 for null terminator */);
    player.nocover_small    = os_malloc(IMAGE_SMALL_WIDTH * IMAGE_SMALL_HEIGHT * GS_PIXEL_DEPTH);
    player.nocover_big      = os_malloc(IMAGE_BIG_WIDTH   * IMAGE_BIG_HEIGHT   * GS_PIXEL_DEPTH);
    player.simage           = os_malloc(IMAGE_SMALL_WIDTH * IMAGE_SMALL_HEIGHT * GS_PIXEL_DEPTH);
    player.bimage           = os_malloc(IMAGE_BIG_WIDTH   * IMAGE_BIG_HEIGHT   * GS_PIXEL_DEPTH);
    player.play_ico         = os_malloc(BROWSER_ICO_WIDTH * BROWSER_ICO_HEIGHT * GS_PIXEL_DEPTH);
    player.pause_ico        = os_malloc(BROWSER_ICO_WIDTH * BROWSER_ICO_HEIGHT * GS_PIXEL_DEPTH);
    player.stop_ico         = os_malloc(BROWSER_ICO_WIDTH * BROWSER_ICO_HEIGHT * GS_PIXEL_DEPTH);
    player.repeat_none_ico  = os_malloc(BROWSER_ICO_WIDTH * BROWSER_ICO_HEIGHT * GS_PIXEL_DEPTH);
    player.repeat_album_ico = os_malloc(BROWSER_ICO_WIDTH * BROWSER_ICO_HEIGHT * GS_PIXEL_DEPTH);
    player.repeat_track_ico = os_malloc(BROWSER_ICO_WIDTH * BROWSER_ICO_HEIGHT * GS_PIXEL_DEPTH);

    win = gs_win_create(MAIN_WINDOW_POSX, MAIN_WINDOW_POSY, MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);
    if (!win)
    {
        DEBUG_EMSG("Failed to create main window");
        goto error;
    }
    gs_win_set_border(win, MAIN_WINDOW_BORDER_WIDTH, MAIN_WINDOW_BORDER_COLOR);
    win->bgcolor.value = MAIN_WINDOW_BGCOLOR;
    player.mwin = win;

    /* artists list */
    {
        memset(&linfo, 0, sizeof(struct gs_wlist_init_info_t));
        linfo.font    = BROWSER_ARTLIST_FONT;
        linfo.x       = BROWSER_ARTLIST_POSX;
        linfo.y       = BROWSER_ARTLIST_POSY;
        linfo.width   = BROWSER_ARTLIST_WIDTH;
        linfo.height  = BROWSER_ARTLIST_HEIGHT;
        linfo.num     = ARTIST_ENTRIES_MAX;
        linfo.maxslen = ARTIST_NAME_MAXLEN;
        player.widget.artlist = gs_wlist_create(win, &linfo);
        if (!player.widget.artlist)
        {
            DEBUG_EMSG("Failed to create list widget for artists");
            goto error;
        }

        gs_wlist_set_colors(player.widget.artlist,
                BROWSER_ALBLIST_FONT_FG_COLOR,
                BROWSER_ALBLIST_SEL_COLOR,
                BROWSER_ALBLIST_CUR_COLOR,
                BROWSER_ALBLIST_CUR_EDGE_COLOR
                );
    }

    /* album list */
    {
        memset(&linfo, 0, sizeof(struct gs_wlist_init_info_t));
        linfo.font    = BROWSER_ALBLIST_FONT;
        linfo.x       = BROWSER_ALBLIST_POSX;
        linfo.y       = BROWSER_ALBLIST_POSY;
        linfo.width   = BROWSER_ALBLIST_WIDTH;
        linfo.height  = BROWSER_ALBLIST_HEIGHT;
        linfo.num     = ALBUM_ENTRIES_MAX;
        linfo.maxslen = ALBUM_NAME_MAXLEN;
        linfo.iwidth  = IMAGE_SMALL_WIDTH;
        linfo.iheight = IMAGE_SMALL_HEIGHT;
        player.widget.alblist = gs_wlist_create(win, &linfo);
        if (!player.widget.artlist)
        {
            DEBUG_EMSG("Failed to create list widget for albums");
            goto error;
        }

        gs_wlist_set_colors(player.widget.alblist,
                BROWSER_ALBLIST_FONT_FG_COLOR,
                BROWSER_ALBLIST_SEL_COLOR,
                BROWSER_ALBLIST_CUR_COLOR,
                BROWSER_ALBLIST_CUR_EDGE_COLOR
                );
    }

    /* track list */
    {
        memset(&linfo, 0, sizeof(struct gs_wlist_init_info_t));
        linfo.font    = BROWSER_TRCKLIST_FONT;
        linfo.x       = BROWSER_TRCKLIST_POSX;
        linfo.y       = BROWSER_TRCKLIST_POSY;
        linfo.width   = BROWSER_TRCKLIST_WIDTH;
        linfo.height  = BROWSER_TRCKLIST_HEIGHT;
        linfo.num     = TRACK_ENTRIES_MAX;
        linfo.maxslen = TRACK_NAME_MAXLEN;
        player.widget.trcklist = gs_wlist_create(win, &linfo);
        if (!player.widget.trcklist)
        {
            DEBUG_EMSG("Failed to create list widget for tracks");
            goto error;
        }

        gs_wlist_set_colors(player.widget.trcklist,
                BROWSER_TRCKLIST_FONT_FG_COLOR,
                BROWSER_TRCKLIST_SEL_COLOR,
                BROWSER_TRCKLIST_CUR_COLOR,
                BROWSER_TRCKLIST_CUR_EDGE_COLOR
                );
    }

    /* progress bar */
    player.widget.pbar = gs_wpbar_create(win,
            BROWSER_PBAR_POSX,
            BROWSER_PBAR_POSY,
            BROWSER_PBAR_WIDTH,
            BROWSER_PBAR_HEIGHT);
    if (!player.widget.pbar)
    {
        DEBUG_EMSG("Failed to create progress bar");
        goto error;
    }

    /* status text */
    player.widget.header = gs_wtext_create(win,
            BROWSER_STATUS_POSX,
            BROWSER_STATUS_POSY,
            BROWSER_STATUS_FONT,
            BROWSER_STATUS_MAXLEN);
    if (!player.widget.header)
    {
        DEBUG_EMSG("Failed to create status text widget");
        goto error;
    }

    /* decode time */
    player.widget.time = gs_wtext_create(win,
            BROWSER_TIME_POSX,
            BROWSER_TIME_POSY,
            BROWSER_TIME_FONT,
            BROWSER_TIME_MAXLEN);
    if (!player.widget.time)
    {
        DEBUG_EMSG("Failed to create decode time text widget");
        goto error;
    }

    /* play, pause, stop pixmap */
    player.widget.ppause = gs_wpixmap_create(win, 
            BROWSER_ICO_POSX,
            BROWSER_ICO_POSY,
            BROWSER_ICO_WIDTH,
            BROWSER_ICO_HEIGHT);
    if (!player.widget.ppause)
    {
        DEBUG_EMSG("Failed to create play/pause/stop widget");
        goto error;
    }

    /* repeat pixmap */
    player.widget.repeat = gs_wpixmap_create(win, 
            BROWSER_REPEAT_POSX,
            BROWSER_REPEAT_POSY,
            BROWSER_REPEAT_WIDTH,
            BROWSER_REPEAT_HEIGHT);
    if (!player.widget.repeat)
    {
        DEBUG_EMSG("Failed to create repeat widget");
        goto error;
    }

    /* progress bar */
    player.widget.volume = gs_wvolume_create(win,
            BROWSER_VOLUME_POSX,
            BROWSER_VOLUME_POSY,
            BROWSER_VOLUME_HEIGHT,
            BROWSER_VOLUME_NBARS,
            BROWSER_VOLUME_BARW,
            BROWSER_VOLUME_GAPW);
    if (!player.widget.volume)
    {
        DEBUG_EMSG("Failed to create volume widget");
        goto error;
    }

    player.qplayer  = os_queue_init(PLAYER_QUEUE_LENGTH, sizeof(struct player_msg_t));
    player.qdecoder = os_queue_init(DECODER_QUEUE_LENGTH, sizeof(struct decoder_msg_t));

    fl_init();
    fl_attach_locks(player_fs_lock, player_fs_unlock);

    return;
error:
    os_wait(OS_WAIT_FOREVER);
}

/*
 *
 */
void player_large_msg(char *st)
{
    struct gs_win_t *win = player.mwin;

    gs_win_clear(win);
    gs_text_put_centered(win, st,
            win->width / 2, win->height / 2, GS_COLOR_YELLOW, win->bgcolor.value, LARGE_MSG_FONT);
    gs_win_refresh(win);

    DEBUG_IMSGF("Player LARGE: ", "sn", st);
}

/*
 *
 * RETURN
 *     1 if valid media present on slot
 */
static int player_wait_media(int state)
{
#define CARD_DETECT_POLL_TO    500
#define CARD_INIT_PAUSE_TO     500
    if (state == MEDIA_INSERT)
    {
        os_wait_ms(CARD_INIT_PAUSE_TO);

        /* wait for card to be inserted */
        while (!card_detect())
            os_wait_ms(CARD_DETECT_POLL_TO);
        DEBUG_IMSG("SD card detect");

        os_wait_ms(CARD_INIT_PAUSE_TO);

        if (sdcard_start() == SD_CARDTYPE_UNUSABLE)
            return 0;

        if (fl_attach_media(media_read, media_write) != FAT_INIT_OK)
        {
            DEBUG_EMSG("media attach failed (SD card)");
            cardstate.card_type = SD_CARDTYPE_UNUSABLE;
            return 0;
        }

        return 1;
    } else {
        /* wait when card will be removed */
        while (card_detect())
            os_wait_ms(CARD_DETECT_POLL_TO);
        DEBUG_IMSG("SD card was removed");
        os_wait_ms(CARD_INIT_PAUSE_TO);
        return 0;
    }
}

/*
 *
 */
static void player_browser()
{
    BASE_TYPE msglen;
    struct player_msg_t msg;
    struct decoder_msg_t dmsg;
    int newmode;

    if (!player_load_sys(&player))
        return;

    if (!player_scan_artists(&player))
        return;

    newmode = PLAYER_MODE_ARTIST_BROWSER;
    player.mode = newmode;
    player_bartist_onenter();

    while (1)
    {
        /*
         * Get event from queue.
         */
        if (os_queue_remove(player.qplayer, OS_FLAG_NONE, OS_MS2TICK(200),
                    &msg, &msglen) == OS_ERR_NONE)
        {
            /*
             * Process event by specific handler.
             */
            switch (player.mode)
            {
                case PLAYER_MODE_ARTIST_BROWSER: newmode = player_bartist_procmsg(&msg); break;
                case PLAYER_MODE_ALBUM_BROWSER:  newmode = player_balbum_procmsg(&msg);  break;
                case PLAYER_MODE_TRACK_BROWSER:  newmode = player_btrack_procmsg(&msg);  break;
                default:
                    /* UNREACHED */
                    DEBUG_EMSGF("unknown mode", "*(1d*)n", player.mode);
            }

            /*
             * Redraw widgets on mode change.
             */
            if (newmode != player.mode)
            {
                switch (newmode)
                {
                    case PLAYER_MODE_ARTIST_BROWSER: player_bartist_onenter(); break;
                    case PLAYER_MODE_ALBUM_BROWSER:  player_balbum_onenter(); break;
                    case PLAYER_MODE_TRACK_BROWSER:  player_btrack_onenter(); break;
                }

                player.mode = newmode;
            }
        }

        /* 
         * Check card.
         */
        if (!card_detect())
        {
            /* stop decoder */
            {
                dmsg.id = DECODER_MSG_ID_STOP;
                DECODER_SEND_MSG(&dmsg, dmsg_empty_t);
            }

            DEBUG_IMSG("Card lost");
            break; /* NOTE */
        }
    }
    player.mode = PLAYER_MODE_NOP;
}

/*
 * load system data (images, ...)
 *
 * RETURN
 *     1 on success, 0 on error
 */
static int player_load_sys()
{
    int rd;
#define NOCOVER_SMALL    "/sys/image/nothumb.pcx"
#define NOCOVER_BIG      "/sys/image/nocover.pcx"
#define PLAY_ICO         "/sys/image/play.pcx"
#define PAUSE_ICO        "/sys/image/pause.pcx"
#define STOP_ICO         "/sys/image/stop.pcx"
#define REPEAT0_ICO      "/sys/image/repeat0.pcx"
#define REPEAT1_ICO      "/sys/image/repeat1.pcx"
#define REPEAT_ICO       "/sys/image/repeat.pcx"

    rd = player_read_full_file(NOCOVER_SMALL);
    if (rd <= 0 || !image_decode(IMAGE_FORMAT_PCX, player.filebuf, rd,
                (uint8*)player.nocover_small, IMAGE_SMALL_WIDTH, IMAGE_SMALL_HEIGHT))
    {
        DEBUG_WMSGF("image read/decode failed", "sn", NOCOVER_SMALL);
    }

    rd = player_read_full_file(NOCOVER_BIG);
    if (rd <= 0 || !image_decode(IMAGE_FORMAT_PCX, player.filebuf, rd,
            (uint8*)player.nocover_big, IMAGE_BIG_WIDTH, IMAGE_BIG_HEIGHT))
    {
        DEBUG_WMSGF("image read/decode failed", "sn", player.path);
    }

    rd = player_read_full_file(PLAY_ICO);
    if (rd <= 0 || !image_decode(IMAGE_FORMAT_PCX, player.filebuf, rd,
            (uint8*)player.play_ico, BROWSER_ICO_WIDTH, BROWSER_ICO_HEIGHT))
    {
        DEBUG_WMSGF("image read/decode failed", "sn", player.path);
    }

    rd = player_read_full_file(PAUSE_ICO);
    if (rd <= 0 || !image_decode(IMAGE_FORMAT_PCX, player.filebuf, rd,
            (uint8*)player.pause_ico, BROWSER_ICO_WIDTH, BROWSER_ICO_HEIGHT))
    {
        DEBUG_WMSGF("image read/decode failed", "sn", player.path);
    }

    rd = player_read_full_file(STOP_ICO);
    if (rd <= 0 || !image_decode(IMAGE_FORMAT_PCX, player.filebuf, rd,
            (uint8*)player.stop_ico, BROWSER_ICO_WIDTH, BROWSER_ICO_HEIGHT))
    {
        DEBUG_WMSGF("image read/decode failed", "sn", player.path);
    }

    rd = player_read_full_file(REPEAT0_ICO);
    if (rd <= 0 || !image_decode(IMAGE_FORMAT_PCX, player.filebuf, rd,
            (uint8*)player.repeat_none_ico, BROWSER_ICO_WIDTH, BROWSER_ICO_HEIGHT))
    {
        DEBUG_WMSGF("image read/decode failed", "sn", player.path);
    }

    rd = player_read_full_file(REPEAT1_ICO);
    if (rd <= 0 || !image_decode(IMAGE_FORMAT_PCX, player.filebuf, rd,
            (uint8*)player.repeat_track_ico, BROWSER_ICO_WIDTH, BROWSER_ICO_HEIGHT))
    {
        DEBUG_WMSGF("image read/decode failed", "sn", player.path);
    }

    rd = player_read_full_file(REPEAT_ICO);
    if (rd <= 0 || !image_decode(IMAGE_FORMAT_PCX, player.filebuf, rd,
            (uint8*)player.repeat_album_ico, BROWSER_ICO_WIDTH, BROWSER_ICO_HEIGHT))
    {
        DEBUG_WMSGF("image read/decode failed", "sn", player.path);
    }

    return 1;
}

/*
 *
 * RETURN
 *     number of bytes readed, 0 on error
 */
static int player_read_full_file(char *path)
{
    int ret;
    FL_FILE *file;

    file = fl_fopen(path, "r");
    ret = 0;
    if (file)
    {
        if (file->filelength <= FILEBUF_SIZE)
        {
            ret = fl_fread(player.filebuf, FILEBUF_SIZE, 1, file);
            if (ret <= 0)
            {
                ret = 0;
                DEBUG_WMSG("read error");
            }
        } else {
            DEBUG_WMSG("file too large");
        }
        fl_fclose(file);
    } else {
        DEBUG_WMSGF("failed to open file", "sn", path);
    }

    return ret;
}

/*
 * scan music directory for artists, fill list with entries
 *
 * TODO
 *     use mutex for FS access
 *
 * RETURN
 *     1 on success, 0 on error
 */
int player_scan_artists()
{
    FL_DIR dirstat;
    struct fs_dir_ent dirent;
    int idx;

    idx = 0;
    gs_wlist_set_active(player.widget.artlist, 0);
    if (fl_opendir(PLAYER_MUSIC_DIR, &dirstat))
    {
        while (fl_readdir(&dirstat, &dirent) == 0)
        {
            if (dirent.is_dir)
            {
                /* list limit was reached, break */
                if (idx >= ARTIST_ENTRIES_MAX)
                {
                    DEBUG_WMSG("artist limit reached");
                    break;
                }
                /* skip ".." and "." */
                if (strcmp(dirent.filename, "..") == 0 || strcmp(dirent.filename, ".") == 0)
                    continue;
                /* skip artist if it's name too long for our list */
                if (strlen(dirent.filename) > ARTIST_NAME_MAXLEN)
                {
#define DEBUG_ART_LEN    8
                    char art[DEBUG_ART_LEN];

                    strncpy(art, dirent.filename, DEBUG_ART_LEN - 1);
                    art[DEBUG_ART_LEN - 1] = 0;
                    DEBUG_WMSGF("artist name too long: ", "s<...>n", art);
                    continue;
                }

                DEBUG_IMSGF("add artist:", "sn", dirent.filename);

                gs_wlist_set_st0(player.widget.artlist, idx, dirent.filename);
                gs_wlist_set_st1(player.widget.artlist, idx, dirent.filename);
                gs_wlist_set_active(player.widget.artlist, idx);
                idx++;
            }
        }

        fl_closedir(&dirstat);

        if (idx == 0)
            return 0;

        /* sort entries */
        gs_wlist_sort(player.widget.artlist);

        return 1;
    }

    DEBUG_WMSG(PLAYER_MUSIC_DIR " directory not found");
    return 0;
}

/*
 * scan artist's directory for albums, fill list with entries
 *
 * TODO
 *     use mutex for FS access
 *
 * RETURN
 *     1 on success, 0 on error
 */
int player_scan_albums(char *artist)
{
    FL_DIR dirstat;
    struct fs_dir_ent dirent;
    int idx;
    char *ptoken[3];
    char *path;

    ptoken[0] = PLAYER_MUSIC_DIR;
    ptoken[1] = artist;
    ptoken[2] = NULL;
    path = player_mkpath(ptoken);
    if (!path)
    {
        DEBUG_WMSG("make path error");
        return 0;
    }
    DEBUG_IMSG("scan albums");

    idx = 0;
    gs_wlist_set_active(player.widget.alblist, 0);
    gs_wlist_set_cur(player.widget.alblist, GS_WLIST_SET_CUR_VALUE, 0);
    if (fl_opendir(path, &dirstat))
    {
        while (fl_readdir(&dirstat, &dirent) == 0)
        {
            if (dirent.is_dir)
            {
                /* list limit was reached, break */
                if (idx >= ALBUM_ENTRIES_MAX)
                {
                    DEBUG_WMSG("album limit reached");
                    break;
                }
                /* skip ".." and "." */
                if (strcmp(dirent.filename, "..") == 0 || strcmp(dirent.filename, ".") == 0)
                    continue;
                /* skip album if it's name too long for our list */
                if (strlen(dirent.filename) > ALBUM_NAME_MAXLEN)
                {
#define DEBUG_ALB_LEN    8
                    char alb[DEBUG_ART_LEN];

                    strncpy(alb, dirent.filename, DEBUG_ALB_LEN - 1);
                    alb[DEBUG_ART_LEN - 1] = 0;
                    DEBUG_WMSGF("album name too long: ", "s<...>n", alb);
                    continue;
                }

                DEBUG_IMSGF("add album:", "sn", dirent.filename);

                gs_wlist_set_st0(player.widget.alblist, idx, dirent.filename);
                gs_wlist_set_st1(player.widget.alblist, idx, dirent.filename);
                gs_wlist_set_active(player.widget.alblist, idx);
                idx++;
            }
        }

        fl_closedir(&dirstat);

        if (idx == 0)
            return 0;

        /* sort entries */
        gs_wlist_sort(player.widget.alblist);

        return 1;
    }

    DEBUG_WMSG("artist directory not found");
    return 0;
}

/*
 * scan directory of album for tracks, fill list with entries
 *
 * TODO
 *     use mutex for FS access
 *
 * RETURN
 *     1 on success, 0 on error
 */
#define EXTENSION_SLEN  4
struct extensions_t {
    char name[EXTENSION_SLEN];
};

static const struct extensions_t extensions[] = {
    {"mp3"},
    {"ogg"},
    {""},
};


int player_scan_tracks(char *artist, char *album)
{
    FL_DIR dirstat;
    struct fs_dir_ent dirent;
    int idx;
    char *ptoken[4];
    char *path;

    ptoken[0] = PLAYER_MUSIC_DIR;
    ptoken[1] = artist;
    ptoken[2] = album;
    ptoken[3] = NULL;
    path = player_mkpath(ptoken);
    if (!path)
    {
        DEBUG_WMSG("make path error");
        return 0;
    }
    DEBUG_IMSG("scan tracks");

    idx = 0;
    gs_wlist_set_active(player.widget.trcklist, 0);
    gs_wlist_set_cur(player.widget.trcklist, GS_WLIST_SET_CUR_VALUE, 0);
    gs_wlist_set_sel(player.widget.trcklist, GS_WLIST_SET_SEL_VALUE, -1);

    if (fl_opendir(path, &dirstat))
    {
        while (fl_readdir(&dirstat, &dirent) == 0)
        {
            if (!dirent.is_dir)
            {
                /* list limit was reached, break */
                if (idx >= TRACK_ENTRIES_MAX)
                {
                    DEBUG_WMSG("track limit reached");
                    break;
                }
                /* skip ".." and "." */
                if (strcmp(dirent.filename, "..") == 0 || strcmp(dirent.filename, ".") == 0)
                    continue;
                /* skip unknown file extensions */
                {
                    int slen;
                    int elen;
                    struct extensions_t *ext;

                    slen = strlen(dirent.filename);
                    ext  = (struct extensions_t *)extensions;
                    while (*ext->name)
                    {
                        elen = strlen(ext->name);
                        if (slen <= elen)
                        {
                            DEBUG_WMSGF("too short file name", "sn", dirent.filename);
                            break;
                        }

                        if (strncmp(&dirent.filename[slen - elen], ext->name, elen) == 0)
                            break;

                        ext++;
                    }
                    if (!*ext->name)
                    {
                        DEBUG_WMSGF("file has unknown extension", "sn", dirent.filename);
                        continue;
                    }
                }

                /* skip track if it's name too long for our list */
                if (strlen(dirent.filename) > TRACK_NAME_MAXLEN)
                {
#define DEBUG_TRCK_LEN    8
                    char trck[DEBUG_TRCK_LEN];

                    strncpy(trck, dirent.filename, DEBUG_TRCK_LEN - 1);
                    trck[DEBUG_TRCK_LEN - 1] = 0;
                    DEBUG_WMSGF("track name too long: ", "s<...>n", trck);
                    continue;
                }

                DEBUG_IMSGF("add track:", "sn", dirent.filename);

                gs_wlist_set_st1(player.widget.trcklist, idx, dirent.filename);
                /*
                 * Remove file extension from displayed value.
                 */
                {
                    int slen;

                    slen = strlen(dirent.filename);
                    if (slen > 4)
                    {
                        /* NOTE spoil dirent.filename */
                        dirent.filename[slen - 4] = 0;
                        gs_wlist_set_st0(player.widget.trcklist, idx, dirent.filename);
                    } else {
                        gs_wlist_set_st0(player.widget.trcklist, idx, dirent.filename);
                    }
                }
                gs_wlist_set_active(player.widget.trcklist, idx);
                idx++;
            }
        }

        fl_closedir(&dirstat);

        if (idx == 0)
            return 0;

        /* sort entries */
        gs_wlist_sort(player.widget.trcklist);

        return 1;
    }

    DEBUG_WMSG("album directory not found");
    return 0;
}

/*
 * scan albums's directory for covers
 *
 * TODO
 *     use mutex for FS access
 *
 * RETURN
 *     1 on success, 0 on error
 */
struct cover_names_t {
#define CONVER_NAME_MAXLEN    16
    char small[CONVER_NAME_MAXLEN];
    char big[CONVER_NAME_MAXLEN];
    int format;
};

const struct cover_names_t cnames[] = {
    {"thumb.pcx", "cover.pcx", IMAGE_FORMAT_PCX},
    {"thumb.jpg", "cover.jpg", IMAGE_FORMAT_JPEG},
    {"", "", 0},
};

int player_scan_thumb(char *artist)
{
    int i;
    char *album;
    struct cover_names_t *cname;
    char *ptoken[5];
    char *path;

    DEBUG_IMSG("load covers");
    for (i = 0; i <= player.widget.alblist->wlist.act; i++)
    {
        album = gs_wlist_get_st1(player.widget.alblist, i);
        if (!album)
        {
            DEBUG_WMSG("entry get error");
            return 0;
        }

        cname = (struct cover_names_t*)cnames;
        while (*cname->small)
        {
            ptoken[0] = PLAYER_MUSIC_DIR;
            ptoken[1] = artist;
            ptoken[2] = album;
            ptoken[3] = cname->small;
            ptoken[4] = NULL;
            path = player_mkpath(ptoken);
            if (!path)
            {
                DEBUG_WMSG("make path error");
                return 0;
            }

            {
                int rd;
                uint32 tm;

                DEBUG_IMSGF("read/decode ", "*\"s*\"n", cname->small);
                rd  = player_read_full_file(player.path);

                if (rd)
                {
                    stimer_settime_us(&tm);
                    if (image_decode(cname->format, player.filebuf, rd,
                            (uint8*)player.simage, IMAGE_SMALL_WIDTH, IMAGE_SMALL_HEIGHT))
                    {
                        tm = stimer_deltatime_us(tm);
                        DEBUG_IMSGF("decode time", "4d< us>n", tm);

                        gs_wlist_set_image(player.widget.alblist, i, player.simage);
                        break; /* NOTE */
                    } else {
                        DEBUG_WMSGF("image decode failed", "sn", player.path);
                    }
                } else {
                    DEBUG_WMSGF("file read failed", "sn", player.path);
                }
            }

            cname++;
        }

        if (*cname->small == 0)
        {
            DEBUG_WMSG("set default thumb");
            gs_wlist_set_image(player.widget.alblist, i, player.nocover_small);
        }

    }

    return 1;
}

/*
 *
 */
int player_get_cover(char *artist, char* album)
{
    struct cover_names_t *cname;
    char *ptoken[5];
    char *path;

    cname = (struct cover_names_t*)cnames;
    while (*cname->small)
    {
        ptoken[0] = PLAYER_MUSIC_DIR;
        ptoken[1] = artist;
        ptoken[2] = album;
        ptoken[3] = cname->big;
        ptoken[4] = NULL;
        path = player_mkpath(ptoken);
        if (!path)
        {
            DEBUG_WMSG("make path error");
            return 0;
        }

        {
            int rd;
            uint32 tm;

            DEBUG_IMSGF("read/decode ", "*\"s*\"n", cname->big);
            rd = player_read_full_file(path);
            if (rd > 0)
            {
                stimer_settime_us(&tm);
                if (image_decode(cname->format, player.filebuf, rd,
                        (uint8*)player.bimage, IMAGE_BIG_WIDTH, IMAGE_BIG_HEIGHT))
                {
                    tm = stimer_deltatime_us(tm);
                    DEBUG_IMSGF("decode time", "4d< us>n", tm);
                    return 1;
                } else {
                    DEBUG_WMSGF("image decode failed", "sn", player.path);
                }
            } else {
                DEBUG_WMSGF("file read failed", "sn", player.path);
            }
        }

        cname++;
    }

    if (*cname->big == 0)
    {
        DEBUG_WMSG("set default cover");
        memcpy(player.bimage, player.nocover_big,
                IMAGE_BIG_WIDTH * IMAGE_BIG_HEIGHT * GS_PIXEL_DEPTH);
    }

    return 1;
}

/*
 * mkpath from tokens
 *
 * ARGS
 *     args    array of tokens, last token should be NULL
 *
 * RETURN
 *     NULL on error, pointer to result path otherwise
 *
 */
char *player_mkpath(char **args)
{
    char *path;
    char **arg;
    int len, slen;

    arg  = args;
    path = player.path;
    len  = 0;
    while (*arg != NULL)
    {
        slen = strlen(*arg);
        len += slen;

        if (len > PLAYER_PATH_MAXLEN)
            return NULL;
        strcpy(path, *arg);
        path += slen;

        arg++;
        if (*arg)
        {
            len += 1;
            if (len > PLAYER_PATH_MAXLEN)
                return NULL;
            *path++ = DIR_CHAR;
        }
    }

    return player.path;
}

/*
 * initialize file cache
 *
 * RETURN
 *     file size of opened file, 0 on error
 */
uint32 player_fcache_open(char *path)
{
    int ret;

    os_mutex_lock(&fcache.mutex, FILE_CACHE_MUTEX, OS_FLAG_NONE, OS_WAIT_FOREVER); 
    fcache.cnt     = 0;
    fcache.wp      = 0;
    fcache.drained = 0;

    fcache.file = fl_fopen(path, "r");
    if (!fcache.file)
    {
        DEBUG_EMSGF("file open error", "sn", path);
        ret = 0;
        goto out;
    }
    ret = fcache.file->filelength;

out:
    os_mutex_unlock(&fcache.mutex, FILE_CACHE_MUTEX);
    return ret;
}

/*
 * close cahed file
 */
void player_fcache_close()
{
    os_mutex_lock(&fcache.mutex, FILE_CACHE_MUTEX, OS_FLAG_NONE, OS_WAIT_FOREVER); 
    if (fcache.file)
        fl_fclose(fcache.file);
    fcache.file = NULL;
    os_mutex_unlock(&fcache.mutex, FILE_CACHE_MUTEX);
}

/*
 * fill cache
 */
void player_fcache_fill()
{
    int rd;
    struct fcache_entry_t *entry;

    os_mutex_lock(&fcache.mutex, FILE_CACHE_MUTEX, OS_FLAG_NONE, OS_WAIT_FOREVER); 

    if (!fcache.file)
        goto out;

    while (fcache.cnt < FILE_CACHE_ENTRIES)
    {
        entry = &fcache.entry[fcache.wp];

        rd = fl_fread(entry->buf, FILE_CACHE_ENTRY_SIZE, 1, fcache.file);
        if (rd <= 0)
        {
            if (rd < 0)
                DEBUG_WMSG("file read error");
            fl_fclose(fcache.file);
            fcache.file    = NULL;
            fcache.drained = 1;
            break;
        }

        entry->len = rd;
        fcache.cnt++;
        fcache.wp++;
        if (fcache.wp >= FILE_CACHE_ENTRIES)
            fcache.wp = 0;
    }

//    os_event_raise(&player.event, FILE_CACHE_FILLED_EVENT);
out:
    os_mutex_unlock(&fcache.mutex, FILE_CACHE_MUTEX);
}

/*
 * remove first entry from cache
 */
void player_fcache_remove()
{
    os_mutex_lock(&fcache.mutex, FILE_CACHE_MUTEX, OS_FLAG_NONE, OS_WAIT_FOREVER); 
    if (fcache.cnt)
        fcache.cnt--;
    os_mutex_unlock(&fcache.mutex, FILE_CACHE_MUTEX);
}

/*
 * get pointer to first entry in cache
 */
struct fcache_entry_t * player_fcache_get(int *drained)
{
    struct fcache_entry_t *ret;
    int rp;

    ret = NULL;
    *drained = 0;
    os_mutex_lock(&fcache.mutex, FILE_CACHE_MUTEX, OS_FLAG_NONE, OS_WAIT_FOREVER); 
    if (fcache.cnt == 0)
    {
        ret = NULL;
        if (fcache.drained)
            *drained = 1;
        goto out;
    }

    if (fcache.wp >= fcache.cnt)
        rp = fcache.wp - fcache.cnt;
    else
        rp = FILE_CACHE_ENTRIES + fcache.wp - fcache.cnt;

    ret = &fcache.entry[rp];
out:
    os_mutex_unlock(&fcache.mutex, FILE_CACHE_MUTEX);
    return ret;
}

/*
 *
 */
static void
player_fs_lock(void)
{
    os_mutex_lock(&fsmutex.mutex, FS_MUTEX_0, OS_FLAG_NONE, OS_WAIT_FOREVER);
    if (fsmutex.cnt == 0 || fsmutex.owner != os_current_taskcb)
    {
        os_mutex_unlock(&fsmutex.mutex, FS_MUTEX_0);

        os_mutex_lock(&fsmutex.mutex, FS_MUTEX_1, OS_FLAG_NONE, OS_WAIT_FOREVER);

        os_mutex_lock(&fsmutex.mutex, FS_MUTEX_0, OS_FLAG_NONE, OS_WAIT_FOREVER);
        fsmutex.owner = os_current_taskcb;
    }
    fsmutex.cnt++;
    os_mutex_unlock(&fsmutex.mutex, FS_MUTEX_0);
}

/*
 *
 */
static void
player_fs_unlock(void)
{
    os_mutex_lock(&fsmutex.mutex, FS_MUTEX_0, OS_FLAG_NONE, OS_WAIT_FOREVER);
    if (fsmutex.cnt == 0)
    {
        DEBUG_EMSG("unlock zero referenced mutex");
        goto out;
    }
    fsmutex.cnt--;
    if (fsmutex.cnt == 0)
    {
        fsmutex.owner = NULL;
        os_mutex_unlock(&fsmutex.mutex, FS_MUTEX_1);
    }
out:
    os_mutex_unlock(&fsmutex.mutex, FS_MUTEX_0);
}

