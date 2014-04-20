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

#ifndef GS_FONT_H
#define GS_FONT_H

#include <types.h>
#include "gs_config.h"

struct gs_font_t {
    int width;    /* width of font */
    int height;   /* height of font in pixels */
    int nsym;     /* number of symbols in font */
    int offset;   /* offset of first implemented character
                   * of ASCII table */
    uint8 *data;  /* pointer to font data */
};

#define GS_RASTER_FONT_COUNT    3

extern struct gs_font_t gs_fonts[GS_RASTER_FONT_COUNT];

#define GS_FONT_9X16   (&gs_fonts[0])
#define GS_FONT_7X8    (&gs_fonts[1])
#define GS_FONT_15X24  (&gs_fonts[2])

void gs_font_init();

#endif

