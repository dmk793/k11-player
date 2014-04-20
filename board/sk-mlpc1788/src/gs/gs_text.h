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

#ifndef GS_TEXT_H
#define GS_TEXT_H

#include <types.h>
#include "gs_font.h"

#define gs_text_put(win, text, x, y, color, bgcolor, font) \
            _gs_text_put(win, text, 0, x, y, color, bgcolor, font)
#define gs_text_put_centered(win, text, x, y, color, bgcolor, font) \
            _gs_text_put_centered(win, text, 0, x, y, color, bgcolor, font)
#define gs_text_nput(win, text, slen, x, y, color, bgcolor, font) \
            _gs_text_put(win, text, slen, x, y, color, bgcolor, font)
#define gs_text_nput_centered(win, text, slen, x, y, color, bgcolor, font) \
            _gs_text_put_centered(win, text, slen, x, y, color, bgcolor, font)


void _gs_text_put(struct gs_win_t *win,
        char *str, int slen, int x, int y, int color, int bgcolor, struct gs_font_t *font);
void _gs_text_put_centered(struct gs_win_t *win,
        char *str, int slen, int x, int y, int color, int bgcolor, struct gs_font_t *font);
int _gs_text_size(char *str, int slen, struct gs_font_t *font, int *width, int *height);

#endif

