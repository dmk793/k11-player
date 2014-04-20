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

#include <types.h>
#include <debug.h>
#include <string.h>
#include "jpeg.h"
#include "tjpgd/tjpgd.h"

#define DEBUG_JPEG

#ifdef DEBUG_JPEG
    #define DPRINT(fmt, ...) dprint(fmt, __VA_ARGS__)
#else
    #define DPRINT(fmt, ...)
#endif

/*
 * Structures used by TJpgDec library
 */
static JDEC jdec;
#define TJPGD_WORK_AREA_SIZE    4096
static uint8 work[TJPGD_WORK_AREA_SIZE];

static UINT jpeg_in_func(JDEC* jdec, BYTE* buff, UINT ndata);
static UINT jpeg_out_func(JDEC* jdec, void* bitmap, JRECT* rect);

/*
 *
 */
static struct jpegdev_t {
    uint8 *src;
    uint32 len;
    uint8 *dst;
    int dstw;
    int dsth;
} jpegdev;

/*
 * convert JPEG image to 24 bpp RGB image (32 bits, bits 24 to 31 unused)
 *
 * ARGS
 *     src   pointer to memory with source JPEG-formated image
 *     len   length of source data
 *     dst   pointer to memory where decoded image will be stored
 *     dstw  maximal width of destination image
 *     dsth  maximal height of destination image
 *
 * RETURN
 *     1 on success, 0 otherwise
 */
#define DST_BPP    4    /* destination BYTES per pixel */
int jpeg_decode(uint8 *src, uint32 len, uint8 *dst, int dstw, int dsth)
{
    int result;

    jpegdev.src  = src;
    jpegdev.len  = len;
    jpegdev.dst  = dst;
    jpegdev.dstw = dstw;
    jpegdev.dsth = dsth;

    /*
     * jd_prepare (
     *     JDEC* jdec,                      // Pointer to blank decompression object
     *     UINT(*infunc)(JDEC*,BYTE*,UINT), // Pointer to input function
     *     void* work,                      // Pointer to work area
     *     UINT sz_work,                    // Size of the work area
     *     void* device                     // Device identifier for the session
     * );
     */
    result = jd_prepare(&jdec, jpeg_in_func, work, TJPGD_WORK_AREA_SIZE, &jpegdev);
    if (result != JDR_OK)
    {
        DPRINT("s1dn", "(E) JPEG prepare, ret code ", result);
        return 0;
    }

    /*
     * JRESULT jd_decomp (
     *     JDEC* jdec,                         // Pointer to valid decompressor object
     *     UINT(*outfunc)(JDEC*,void*,JRECT*), // Pointer to output function
     *     BYTE scale                          // Scaling factor
     * );
     */
    result = jd_decomp(&jdec, jpeg_out_func, 0);   /* Start to decompress with 1/1 scaling */
    if (result == JDR_OK) {
        return 1;
    } else {
        DPRINT("s1dn", "(E) JPEG decomp, ret code ", result);
        return 0;
    }    

    return 1;
}

/*
 * This function is the data input interface of the TJpgDec module. The corresponding
 * decompression session can be identified by the pointer to the device identifier
 * jdec->device passed to the 5th argument of jd_prepare function.
 *
 * ARGS
 *     jdec    Pointer to the decompression object
 *     buff    Pointer to buffer to store the read data
 *     ndata   Number of bytes to read
 *
 * RETURN
 *     Returns number of bytes read/removed. When a zero is returned, the
 *     jd_prepare and jd_decomp function aborts with JDR_INP.
 */
static UINT jpeg_in_func(JDEC* jdec, BYTE* buff, UINT ndata)
{
    static struct jpegdev_t *dev;
    uint32 n;

    dev = (struct jpegdev_t*)jdec->device;   /* Device identifier for the session (5th argument of jd_prepare function) */

//    dprint("<len = >4d<, ndata = >4d<, buff >sn", dev->len, ndata, buff ? "" : "NULL");
//    dprint("<len = >4dn", dev->len);

    if (buff) {
        if (dev->len > ndata)
            n = ndata;
        else
            n = dev->len;

        if (n)
            memcpy(buff, dev->src, n);

        dev->len -= n;
        dev->src += n;

        return n;
    } else {
        if (dev->len > ndata)
            n = ndata;
        else
            n = dev->len;

        dev->len -= n;
        dev->src += n;

        return n;
    }    

    return 0;
}

/*
 * This function is the data output interface of the TJpgDec module. The
 * corresponding decompression session can be identified by the pointer to
 * the device identifier jdec->device passed to the 5th argument of jd_prepare
 * function.
 *
 * The bitmap is sent to the frame buffer or display device in this function.
 * The first pixel in the bitmap is the left-top of the rectangular, the
 * second one is next right and last pixel is the bottom-right of the rectangular.
 *
 * The size of rectangular varies from 1x1 to 16x16 depends on clipping,
 * scaling and sampling factor of the image. If the rectangular is out
 * of the frame buffer, it should be clipped in this function.
 *
 * The pixel format is defined by JD_FORMAT parameter in the configuration
 * option. When it is configured to RGB888, the bitmap is a BYTE array holds
 * the RGB data in 3 bytes per pixel: RRRRRRRR, GGGGGGGG, BBBBBBBB, RRRRRRRR,
 * GGGGGGGG, BBBBBBBB, ... When it is configured to RGB565, the bitmap
 * is a WORD array holds the RGB data in 1 word per pixel: RRRRRGGGGGGBBBBB,
 * RRRRRGGGGGGBBBBB, RRRRRGGGGGGBBBBB, ...
 *
 * ARGS
 *
 *     jdec    Pointer to the decompression object
 *     bitmap  RGB bitmap to be output
 *     rect    Rectangular region to output
 *
 * RETURN
 *     Normally returns 1. It lets TJpgDec to continue the decompressing
 *     process. When a 0 is returned, the jd_decomp function aborts with JDR_INTR.
 *     This is useful to interrupt the decompression process.
 *
 *
 */
static UINT jpeg_out_func(JDEC* jdec, void* bitmap, JRECT* rect)
{
    int lines, pix, srcw;
    static struct jpegdev_t *dev;
    uint8 *src;
    uint8 *dst;

    dev = (struct jpegdev_t*)jdec->device;   /* Device identifier for the session (5th argument of jd_prepare function) */

    if (rect->right > dev->dstw || rect->bottom > dev->dsth)
    {
        DPRINT("sn", "(E) JPEG decode, source and distination size mismatch");
        DPRINT("ts *s_g2d _*d_g2dn", "width : ", 6, rect->right, 6, dev->dstw);
        DPRINT("ts *s_g2d _*d_g2dn", "height: ", 6, rect->right, 6, dev->dsth);
        return 0;
    }

#define COLOR_DEPTH    4
    src = (uint8*)bitmap;
    dst = &dev->dst[COLOR_DEPTH * (dev->dstw * rect->top + rect->left)];
    lines = rect->bottom - rect->top + 1;
    srcw  = rect->right - rect->left + 1;

    while (lines--)
    {
        pix = srcw;
        while (pix--)
        {
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = 0xff;
        }
        dst += (dev->dstw - srcw) * COLOR_DEPTH;
    }

    return 1;
}

