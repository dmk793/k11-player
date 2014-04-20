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

#ifndef GS_WTEXT_H
#define GS_WTEXT_H

struct gs_widget_t *gs_wtext_create(struct gs_win_t *win,
    int x, int y, struct gs_font_t *font, int slen);
void gs_wtext_destroy(struct gs_widget_t *wd);
void gs_wtext_set_colors(struct gs_widget_t *wd, int fg, int bg);
void gs_wtext_set_text(struct gs_widget_t *wd, char *s);

#define GS_WTEXT_DRAW_CENTERED    1
#define GS_WTEXT_DRAW_LEFT        0
void gs_wtext_draw(struct gs_widget_t *wd, int align);

#endif

