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

#ifndef NVRAM_H
#define NVRAM_H

#include <types.h>

#pragma pack(push, 1)

struct nvram_store_t {
    uint32 service;     /* reserved for DPort */
    uint32 artist_hash; /* last playback artist name hash */
    uint32 album_hash;  /* last playback album name hash */
    uint32 track_hash;  
    uint32 reserved;
};

#pragma pack(pop)

void nvram_load();
void nvram_save();

extern struct nvram_store_t nvstore;

#endif

