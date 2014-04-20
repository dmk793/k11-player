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

#include "net_def.h"

/*
 *
 */
uint32 NET_HTONL(uint32 n32)
{
    uint32 ret;

    ret  = ((n32 >> 24) & 0xff) << 0;
    ret |= ((n32 >> 16) & 0xff) << 8;
    ret |= ((n32 >>  8) & 0xff) << 16;
    ret |= ((n32 >>  0) & 0xff) << 24;

    return ret;
}

/*
 *
 */
uint16 NET_HTONS(uint16 n16)
{
    return ((uint16)((n16 << 8) | (n16 >> 8)));
}

