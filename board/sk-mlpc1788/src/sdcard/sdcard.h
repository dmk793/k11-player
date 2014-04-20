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

#ifndef SD_CARD_H
#define SD_CARD_H

#include <types.h>
#include <os.h>
#include "sdcard_hw.h"


struct card_state_t {
    union {
        struct card_csd_v10_t v10;
        struct card_csd_v20_t v20;
    } csd;
    uint16 rca;

    int card_type;
    uint32 blocklen;
};

extern struct card_state_t cardstate;

#define SD_CARDTYPE_UNUSABLE       0x00
#define SD_CARDTYPE_SD_V1X         0x01
#define SD_CARDTYPE_SD_V200_SC     0x02
#define SD_CARDTYPE_SD_V200_HC     0x03
#define SD_CARDTYPE_SD_V200_XC     0x04

//void sdcard_task();
int sdcard_start();
int sdcard_read(uint32 bnum, uint8 *data);

int media_read(uint32 sector, uint8 *buffer, uint32 sector_count);
int media_write(uint32 sector, uint8 *buffer, uint32 sector_count);

#endif

