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
 */
#include <types.h>
#include <debug.h>
#include "player_bartist.h"
#include "player_layout.h"
#include "../buttons.h"
#include "../gs/gs.h"

#define DEBUG_BARTIST

#ifdef DEBUG_BARTIST
    #define DPRINT(fmt, ...) dprint(fmt, __VA_ARGS__)
#else
    #define DPRINT(fmt, ...)
#endif

/*
 * refresh widgets on entry
 */
void player_bartist_onenter()
{
    gs_win_clear(player.mwin);
    gs_wlist_draw(player.widget.artlist);
    gs_win_refresh(player.mwin);
    /* flush queue (remove odd keypress by user) */
    PLAYER_QUEUE_FLUSH(player);
}

/*
 * process message
 */
int player_bartist_procmsg(struct player_msg_t *msg)
{
    uint32 button;
    switch (msg->id)
    {
        case PLAYER_MSG_ID_BUTTON:
            button = msg->button.code;

#define BIG_SCROLL    10
            if (BUTTON_PRESSED(button, BUTTON_UP) ||
                BUTTON_PRESSED(button, BUTTON_PGUP))
            {
                gs_wlist_set_cur(player.widget.artlist, GS_WLIST_SET_CUR_PREV,
                        BUTTON_PRESSED(button, BUTTON_UP) ? 1 : BIG_SCROLL);

                gs_wlist_draw(player.widget.artlist);
                gs_win_refresh(player.mwin);
            } else if (BUTTON_PRESSED(button, BUTTON_DOWN) ||
                       BUTTON_PRESSED(button, BUTTON_PGDOWN)) {

                gs_wlist_set_cur(player.widget.artlist, GS_WLIST_SET_CUR_NEXT,
                        BUTTON_PRESSED(button, BUTTON_DOWN) ? 1 : BIG_SCROLL);

                gs_wlist_draw(player.widget.artlist);
                gs_win_refresh(player.mwin);
            } else if (BUTTON_PRESSED(button, BUTTON_ENTER)) {
                char *artist;

                artist = GS_WLIST_GET_CUR_ST1(player.widget.artlist);
                if (artist)
                {
                    DPRINT("<artist \">s<\" selected>n", artist);
                    player_large_msg(PLAYER_INFOMSG_PLEASE_WAIT);

                    if (player_scan_albums(artist))
                    {
                        player_scan_thumb(artist);
                        return PLAYER_MODE_ALBUM_BROWSER;
                    }

                    player_large_msg("No albums");
                    os_wait_ms(1000);
                    player_bartist_onenter();
                }
            }

            break;
    }

    /* unchanged */
    return player.mode;
}

