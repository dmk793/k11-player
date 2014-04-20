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
#include "pcx.h"

#define DEBUG_PCX

#ifdef DEBUG_PCX
    #define DPRINT(fmt, ...) dprint(fmt, __VA_ARGS__)
#else
    #define DPRINT(fmt, ...)
#endif

#define DEBUG_PREFIX "PCX "
#define DPREFIX     DEBUG_PREFIX
#define EPREFIX     ERR_PREFIX""DEBUG_PREFIX
#define WPREFIX     ERR_PREFIX""DEBUG_PREFIX



#pragma pack(push, 0)
struct pcx_head_t {
  uint8     Identifier;     /* 0x0000, PCX Id Number (Always 0x0A) */
  uint8     Version;        /* 0x0001, Version Number
                             *             0    PC Paintbrush v2.5
                             *             2    PC Paintbrush v2.8 w palette information
                             *             3    PC Paintbrush v2.8 w/o palette information
                             *             4    PC Paintbrush/Windows
                             *             5    PC Paintbrush v3.0+
                             */
  uint8     Encoding;       /* 0x0002, Encoding Format, 1 = RLE, none other known */
  uint8     BitsPerPixel;   /* 0x0003, Bits per Pixel */
  uint16    XStart;         /* 0x0004, Left of image */
  uint16    YStart;         /* 0x0006, Top of Image */
  uint16    XEnd;           /* 0x0008, Right of Image */
  uint16    YEnd;           /* 0x000A, Bottom of image */
  uint16    HorzRes;        /* 0x000C, Horizontal Resolution */
  uint16    VertRes;        /* 0x000E, Vertical Resolution */
  uint8     Palette[48];    /* 0x0010, 16-Color EGA Palette, 16 RGB triplets */
  uint8     Reserved1;      /* 0x0040, Reserved (Always 0) */
  uint8     NumBitPlanes;   /* 0x0041, Number of Bit Planes */
  uint16    BytesPerLine;   /* 0x0042, Bytes per Scan-line */
  uint16    PaletteType;    /* 0x0044, Palette Type 
                             *             1    color/bw palette
                             *             2    grayscale image
                             */
  /* NOTE
   * HorzScreenSize and VertScreenSize were added to the PCX format
   * starting with PC Paintbrush 4.0 and 4.0 Plus.
   * There is no way to know if these fields contain valid
   * information or are part of the Reserved2 field.
   */
  uint16    HorzScreenSize; /* 0x0046, Horizontal Screen Size */
  uint16    VertScreenSize; /* 0x0048, Vertical Screen Size */
  uint8     Reserved2[54];  /* 0x004a, Reserved (Always 0) */
};
#pragma pack(pop)


/*
 * PCX image data are compressed using run-length encoding (RLE), a simple 
 * lossless compression algorithm that collapses a series of three or more 
 * consecutive bytes with identical values into a two-byte pair. The two most-
 * significant bits of a byte are used to determine whether the given data 
 * represent a single pixel of a given palette index or color value, or an RLE 
 * pair representing a series of several pixels of a single value:
 * 
 *     if both bits are 1, the byte is interpreted as the run length. This leaves 
 *     6 bits for the actual run length value, i.e. a value range of 0-63 in any
 *     other case, the byte is interpreted as a single pixel value. This 
 *     leaves all value for which bit #7 and bit #8 are not 1 at the same time.
 *     This requirement is not met by all values of 192 (binary 11000000) and above.
 * 
 * Due to the use of the two most-significant bits as flags, pixel values from 
 * 192 to 255 (with their most-significant bit already set) must be stored in an 
 * RLE byte pair, even when they only occur one or two pixels in succession, 
 * whereas color indexes 0 to 191 can be stored directly or in RLE byte pairs (
 * whichever is more space-efficient).
 */

/*
 * convert PCX image to 24 bpp RGB image (32 bits, bits 24 to 31 unused)
 *
 * Only following PCX images supported
 *    BitsPerPixel    8
 *    NumBitPlanes    3
 *
 * ARGS
 *     src   pointer to memory with source PCX-formated image
 *     len   length of source data
 *     dst   pointer to memory where decoded image will be stored
 *     dstw  maximal width of destination image
 *     dsth  maximal height of destination image
 *
 * RETURN
 *     1 on success, 0 otherwise
 */
#define DST_BPP    4    /* destination BYTES per pixel */
int pcx_decode(uint8 *src, uint32 len, uint8 *dst, int dstw, int dsth)
{
    int x, y, p, r, rlelen;
    struct pcx_head_t *head;
    uint16 hres, vres;

    if (len < sizeof(struct pcx_head_t))
    {
        DPRINT("sn", EPREFIX"decode, not enough data for header");
        return 0;
    }

    head = (struct pcx_head_t*)src;
    if (head->Identifier != 0x0A)
    {
        DPRINT("sn", EPREFIX"decode, no valid header found");
        return 0;
    }
    if (head->Version < 5)
    {
        DPRINT("s1dsn", EPREFIX"decode, version ", head->Version, " not supported");
        return 0;
    }
    if (head->BitsPerPixel != 8)
    {
        DPRINT("s1dsn", EPREFIX"decode, BitsPerPixel ", head->BitsPerPixel, " not supported");
        return 0;
    }
    if (head->NumBitPlanes != 3)
    {
        DPRINT("s1dsn", EPREFIX"decode, NumBitPlanes ", head->NumBitPlanes, " not supported");
        return 0;
    }
    if (head->PaletteType != 1)
    {
        DPRINT("s1dsn", EPREFIX"decode, PaletteType ", head->PaletteType, " not supported");
        return 0;
    }

    hres = head->XEnd - head->XStart + 1;
    vres = head->YEnd - head->YStart + 1;
    if (hres > dstw || vres > dsth)
    {
        DPRINT("sn", EPREFIX"decode, source to large for destination");
        DPRINT("ts *s_g2d _*d_g2dn", "width : ", 6, hres, 6, dstw);
        DPRINT("ts *s_g2d _*d_g2dn", "height: ", 6, vres, 6, dsth);
        return 0;
    }

    /* move source pointer to image data */
    src += sizeof(struct pcx_head_t);
    len -= sizeof(struct pcx_head_t);

    /* TODO */
    y = 0; /* Y coordinate of destination image */
    x = 0; /* X coordinate of destination image */
    p = 0; /* current plane (R, G, B) */
    while (len)
    {
        if ((*src & 0xc0) == 0xc0)
        {
            rlelen = *src & 0x3f;
            src++;
            len--;
        } else {
            rlelen = 1;
        }

        for (r = 0; r < rlelen; r++)
        {
//            dprint("s1x_s4dn","src = ", *src, "len = ", len);
            dst[(y * dstw + x) * DST_BPP + p] = *src;
            x++;
            if (x >= hres)
            {
                x = 0;
                p++;
                if (p >= 3)
                {
                    p = 0;
                    y++;
                    if (y >= dsth)
                        break;
                }
            }
        }
        src++;
        len--;
        if (y >= dsth && len)
        {
            DPRINT("s_4dsn", EPREFIX"decode, conversion error, ", len, " bytes left");
            return 0;
        }
    }

    return 1;
}

