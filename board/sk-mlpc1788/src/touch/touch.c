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
#include "../gs/gs.h"
#include "touch_hw.h"
#include "touch.h"
#include "../player/player.h"

/*
 *
 */
void touch_task()
{
    uint16 x;
    uint16 y;
    struct player_msg_t msg;

    touch_hw_init();
    while (1)
    {
        os_event_wait(&touch_event, TOUCH_EVENT_MASK_IRQ, OS_FLAG_CLEAR, OS_WAIT_FOREVER);
        if (touch_hw_getvalues(GS_RES_HORIZONTAL, GS_RES_VERTICAL, &x, &y))
        {
            msg.id = PLAYER_MSG_ID_TOUCH;
            msg.data.touch.x = x;
            msg.data.touch.y = y;
            os_queue_add(player.qmsg, OS_FLAG_NOWAIT, OS_WAIT_FOREVER, &msg, PLAYER_MSG_SIZE(pmsg_touch_t));
        }
    }
}

