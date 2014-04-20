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

#ifndef GS_WPBAR_H
#define GS_WPBAR_H

#include <types.h>
#include "../gs.h"

struct gs_widget_t *gs_wpbar_create(struct gs_win_t *win,
    int x, int y, int width, int height);
void gs_wpbar_set_colors(struct gs_widget_t *wd, int fg, int bg, int b);
void gs_wpbar_set_value(struct gs_widget_t *wd, int value);
void gs_wpbar_draw(struct gs_widget_t *wd);

#endif

