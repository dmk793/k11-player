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

#ifndef DMA_H
#define DMA_H

#include <types.h>

void dma_init();
int dma_copy_window(void *dst, uint32 dstwidth, void *src, uint32 srclen, uint32 srcwidth, uint32 lines);
int dma_wait_chan(int chmask, uint32 to);
int dma_sd_read(void *dst);
void dma_sd_cancel();

#define DMA_REQUEST_SD    1

#define DMA_CHMASK_WINDOW   0x80
#define DMA_CHMASK_SD       0x40

#endif

