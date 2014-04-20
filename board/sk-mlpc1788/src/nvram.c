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

#include <LPC177x_8x.h>
#include <LPC177x_8x_bits.h>
#include <debug.h>
#include "nvram.h"

struct nvram_store_t nvstore;

//            if (LPC_RTC->GPREG0 == 0x4444BEEF)

/*
 *
 */
void nvram_load()
{
    nvstore.service     = LPC_RTC->GPREG0;
    nvstore.artist_hash = LPC_RTC->GPREG1;
    nvstore.album_hash  = LPC_RTC->GPREG2;
    nvstore.track_hash  = LPC_RTC->GPREG3;
    nvstore.reserved    = LPC_RTC->GPREG4;
}

/*
 *
 */
void nvram_save()
{
//    LPC_RTC->GPREG0 = nvstore.service;
    LPC_RTC->GPREG1 = nvstore.artist_hash;
    LPC_RTC->GPREG2 = nvstore.album_hash;
    LPC_RTC->GPREG3 = nvstore.track_hash;
    LPC_RTC->GPREG4 = nvstore.reserved;
}

