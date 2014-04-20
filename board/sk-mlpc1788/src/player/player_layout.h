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

#ifndef PLAYER_LAYOUT_H
#define PLAYER_LAYOUT_H

#define IMAGE_SMALL_WIDTH    120
#define IMAGE_SMALL_HEIGHT   120
#define IMAGE_BIG_WIDTH      200
#define IMAGE_BIG_HEIGHT     200

#define MAIN_WINDOW_POSX          0
#define MAIN_WINDOW_POSY          0
#define MAIN_WINDOW_WIDTH         GS_RES_HORIZONTAL
#define MAIN_WINDOW_HEIGHT        GS_RES_VERTICAL
#define MAIN_WINDOW_BGCOLOR       GS_WIN_DEFAULT_COLOR
#define MAIN_WINDOW_BORDER_WIDTH  0
#define MAIN_WINDOW_BORDER_COLOR  GS_COLOR_WHITE

#define BROWSER_WIDGET_PAD                 2

#define FONT_COLOR                0xff88a088
#define CUR_COLOR                 0xff88ff88
#define CUR_EDGE_COLOR            0xffff8080


#define BROWSER_ARTLIST_FONT                GS_FONT_15X24
#define BROWSER_ARTLIST_POSX                BROWSER_WIDGET_PAD
#define BROWSER_ARTLIST_POSY                BROWSER_WIDGET_PAD
#define BROWSER_ARTLIST_WIDTH               (MAIN_WINDOW_WIDTH  - BROWSER_WIDGET_PAD * 2)
#define BROWSER_ARTLIST_HEIGHT              (MAIN_WINDOW_HEIGHT - BROWSER_WIDGET_PAD * 2)
#define BROWSER_ARTLIST_FONT_FG_COLOR       FONT_COLOR
#define BROWSER_ARTLIST_SEL_COLOR           0xff082280
#define BROWSER_ARTLIST_CUR_COLOR           CUR_COLOR
#define BROWSER_ARTLIST_CUR_EDGE_COLOR      CUR_EDGE_COLOR

#define BROWSER_ALBLIST_PADY                0

#define BROWSER_ALBLIST_FONT                GS_FONT_15X24
#define BROWSER_ALBLIST_POSX                BROWSER_WIDGET_PAD
#define BROWSER_ALBLIST_POSY                BROWSER_ALBLIST_PADY
#define BROWSER_ALBLIST_WIDTH               (MAIN_WINDOW_WIDTH  - BROWSER_WIDGET_PAD * 2)
#define BROWSER_ALBLIST_HEIGHT              (MAIN_WINDOW_HEIGHT - BROWSER_ALBLIST_PADY * 2)
#define BROWSER_ALBLIST_FONT_FG_COLOR       FONT_COLOR
#define BROWSER_ALBLIST_SEL_COLOR           0xff082280
#define BROWSER_ALBLIST_CUR_COLOR           CUR_COLOR
#define BROWSER_ALBLIST_CUR_EDGE_COLOR      CUR_EDGE_COLOR

#define IMAGE_BIG_PAD                       4

#define BROWSER_REPEAT_POSX                 8   
#define BROWSER_REPEAT_POSY                 10 
#define BROWSER_REPEAT_WIDTH                16
#define BROWSER_REPEAT_HEIGHT               16

#define BROWSER_STATUS_POSX                 (4 * IMAGE_BIG_PAD + BROWSER_REPEAT_WIDTH)
#define BROWSER_STATUS_POSY                 10
#define BROWSER_STATUS_FONT                 GS_FONT_9X16
#define BROWSER_STATUS_MAXLEN               ((MAIN_WINDOW_WIDTH - (4 * IMAGE_BIG_PAD) - BROWSER_REPEAT_WIDTH) / BROWSER_STATUS_FONT->width)

#define BROWSER_TRCKLIST_FONT               GS_FONT_9X16
#define BROWSER_TRCKLIST_POSX               (IMAGE_BIG_WIDTH + 2 * IMAGE_BIG_PAD)
#define BROWSER_TRCKLIST_POSY               ((MAIN_WINDOW_HEIGHT - IMAGE_BIG_WIDTH) / 2)
#define BROWSER_TRCKLIST_WIDTH              (MAIN_WINDOW_WIDTH  - ((IMAGE_BIG_WIDTH + 2 * IMAGE_BIG_PAD)) - IMAGE_BIG_PAD)
#define BROWSER_TRCKLIST_HEIGHT             IMAGE_BIG_WIDTH
#define BROWSER_TRCKLIST_FONT_FG_COLOR      FONT_COLOR
#define BROWSER_TRCKLIST_SEL_COLOR          0xff082280
#define BROWSER_TRCKLIST_CUR_COLOR          CUR_COLOR
#define BROWSER_TRCKLIST_CUR_EDGE_COLOR     CUR_EDGE_COLOR

#define BROWSER_VOLUME_POSX                 IMAGE_BIG_PAD
#define BROWSER_VOLUME_POSY                 (IMAGE_BIG_WIDTH + (MAIN_WINDOW_HEIGHT - IMAGE_BIG_HEIGHT)/2 + IMAGE_BIG_PAD + 8)
#define BROWSER_VOLUME_NBARS                10
#define BROWSER_VOLUME_BARW                 6
#define BROWSER_VOLUME_GAPW                 1
#define BROWSER_VOLUME_WIDTH                ((BROWSER_VOLUME_BARW + BROWSER_VOLUME_GAPW) * BROWSER_VOLUME_NBARS)
#define BROWSER_VOLUME_HEIGHT               16

#define BROWSER_PBAR_POSX                   (IMAGE_BIG_PAD + BROWSER_VOLUME_POSX + BROWSER_VOLUME_WIDTH)
#define BROWSER_PBAR_POSY                   (IMAGE_BIG_WIDTH + (MAIN_WINDOW_HEIGHT - IMAGE_BIG_HEIGHT)/2 + IMAGE_BIG_PAD + 8)
#define BROWSER_PBAR_WIDTH                  (BROWSER_TIME_POSX - (IMAGE_BIG_PAD * 4) - BROWSER_ICO_WIDTH - BROWSER_VOLUME_WIDTH)
#define BROWSER_PBAR_HEIGHT                 16

#define BROWSER_ICO_POSX                   (BROWSER_PBAR_POSX + BROWSER_PBAR_WIDTH + IMAGE_BIG_PAD)
#define BROWSER_ICO_POSY                   BROWSER_PBAR_POSY - 1
#define BROWSER_ICO_WIDTH                  16
#define BROWSER_ICO_HEIGHT                 16

#define BROWSER_TIME_MAXLEN                 9
#define BROWSER_TIME_POSX                   (MAIN_WINDOW_WIDTH - IMAGE_BIG_PAD - (BROWSER_TIME_FONT->width * BROWSER_TIME_MAXLEN))
#define BROWSER_TIME_POSY                   (IMAGE_BIG_WIDTH + (MAIN_WINDOW_HEIGHT - IMAGE_BIG_HEIGHT)/2 + IMAGE_BIG_PAD + 8)
#define BROWSER_TIME_FONT                   GS_FONT_9X16

#endif

