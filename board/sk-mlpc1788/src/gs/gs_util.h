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

#ifndef GS_UTIL_H
#define GS_UTIL_H

#include <types.h>
#include "gs.h"

struct yuv_color_t {
    uint8 y;
    uint8 u;
    uint8 v;
};

void gs_util_rgb2yuv(union gs_pixel_t *rgb, struct yuv_color_t *yuv);
void gs_util_yuv2rgb(struct yuv_color_t *yuv, union gs_pixel_t *rgb);

#endif

