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
#include "../gs/gs.h"
#include "pcx.h"
#include "jpeg.h"
#include "image.h"

#define DEBUG_IMAGE

#ifdef DEBUG_IMAGE
    #define DPRINT(fmt, ...) dprint(fmt, __VA_ARGS__)
#else
    #define DPRINT(fmt, ...)
#endif

#define DEBUG_PREFIX "Image, "
#define DPREFIX     DEBUG_PREFIX
#define EPREFIX     ERR_PREFIX""DEBUG_PREFIX
#define WPREFIX     ERR_PREFIX""DEBUG_PREFIX

/*
 * decode image to 24 bpp RGB image (32 bits per pixel actually, bits 24 to 31 unused)
 *
 * ARGS
 *     src   pointer to memory with source image
 *     len   length of source data
 *     dst   pointer to memory where decoded image will be stored
 *     dstw  maximal width of destination image
 *     dsth  maximal height of destination image
 *
 * RETURN
 *     1 on success, 0 on error
 *
 */
int image_decode(int format, uint8 *src, uint32 len, uint8 *dst, int dstw, int dsth)
{
    switch (format)
    {
        case IMAGE_FORMAT_PCX:
            return pcx_decode(src, len, dst, dstw, dsth);
        case IMAGE_FORMAT_JPEG:
            return jpeg_decode(src, len, dst, dstw, dsth);
    }

    dprint("s4dn", ERR_PREFIX"unknown image format, ", format);
    return 0;
}

/*
 * get average color of image (color of edges)
 *
 * 24 bpp
 *
 * ARGS
 *     data    pointer to image's data
 *     w       width of data
 *     h       height of data
 *     bw      edge width
 */
uint32 image_get_avcolor(uint8 *data, int w, int h, int bw)
{
    uint32 nx, ny, n;
    uint32 avr, avg, avb;
    union gs_pixel_t avcolor;
    union gs_pixel_t *pixel;

    if (bw > (w / 2) || bw > (h / 2))
        return 0;

    pixel = (union gs_pixel_t *)data;
    avr   = 0;
    avg   = 0;
    avb   = 0;

    /* top */
    ny = bw;
    while (ny--)
    {
        nx = w;
        while (nx--)
        {
            avr += pixel->red;
            avg += pixel->green;
            avb += pixel->blue;

            pixel++;
        }
    }

    /* left and right */
    ny = h - (bw * 2);
    while (ny--)
    {
        nx = bw;
        while (nx--)
        {
            avr += pixel->red;
            avg += pixel->green;
            avb += pixel->blue;

            pixel++;
        }
        pixel += w - (bw * 2);

        nx = bw;
        while (nx--)
        {
            avr += pixel->red;
            avg += pixel->green;
            avb += pixel->blue;

            pixel++;
        }
    }

    /* bottom */
    ny = bw;
    while (ny--)
    {
        nx = w;
        while (nx--)
        {
            avr += pixel->red;
            avg += pixel->green;
            avb += pixel->blue;

            pixel++;
        }
    }

    n  = 2 * bw * w;
    n += 2 * bw * (h - (bw * 2));

    avcolor.red   = avr / n;
    avcolor.green = avg / n;
    avcolor.blue  = avb / n;

    return (avcolor.value & 0x00ffffff);
}

