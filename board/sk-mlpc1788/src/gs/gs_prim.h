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

#ifndef GS_PRIM_H
#define GS_PRIM_H

#include <types.h>
#include "gs.h"

void gs_prim_draw_rect(struct gs_win_t *win,
        int x0, int y0, int x1, int y1, int lwidth, uint32 color);
void gs_prim_fill_rect(struct gs_win_t *win,
        int x0, int y0, int x1, int y1, uint32 color);
#endif

