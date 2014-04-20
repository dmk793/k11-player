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
#include "gs.h"

/*
 * From "Video Demystified, Fourth Edition",
 * RGB - YCbCr Equations: SDTV, Computer Systems Considerations
 *
 * Y  =      (0.257 * R) + (0.504 * G) + (0.098 * B) + 16
 * Cb = U = -(0.148 * R) - (0.291 * G) + (0.439 * B) + 128
 * Cr = V =  (0.439 * R) - (0.368 * G) - (0.071 * B) + 128
 *
 * R = 1.164(Y - 16) + 1.596(V - 128)
 * G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
 * B = 1.164(Y - 16)                  + 2.018(U - 128)
 *
 */

/*
 * RGB -> YUV
 */
void gs_util_rgb2yuv(union gs_pixel_t *rgb, struct yuv_color_t *yuv)
{
    int y, u, v;
    y = ( (257 * rgb->red) + (504 * rgb->green) + ( 98 * rgb->blue)) / 1000 + 16 ;
    u = (-(148 * rgb->red) - (291 * rgb->green) + (439 * rgb->blue)) / 1000 + 128;
    v = ( (439 * rgb->red) - (368 * rgb->green) - ( 71 * rgb->blue)) / 1000 + 128;

    yuv->y = y > 255 ? 255 : (y < 0 ? 0 : y);
    yuv->u = u > 255 ? 255 : (u < 0 ? 0 : u);
    yuv->v = v > 255 ? 255 : (v < 0 ? 0 : v);
}

/*
 * YUV -> RGB
 */
void gs_util_yuv2rgb(struct yuv_color_t *yuv, union gs_pixel_t *rgb)
{
    int r, g, b;
    int y, u, v;

    y = yuv->y;
    u = yuv->u;
    v = yuv->v;

    r = (1164 * (y - 16) + 1596 * (v - 128)                   ) / 1000;
    g = (1164 * (y - 16) -  813 * (v - 128) -  391 * (u - 128)) / 1000;
    b = (1164 * (y - 16)                    + 2018 * (u - 128)) / 1000;

    rgb->red   = r > 255 ? 255 : (r < 0 ? 0 : r);
    rgb->green = g > 255 ? 255 : (g < 0 ? 0 : g);
    rgb->blue  = b > 255 ? 255 : (b < 0 ? 0 : b);
}

