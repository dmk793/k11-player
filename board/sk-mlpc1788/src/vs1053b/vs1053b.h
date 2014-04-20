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

#ifndef VS1053B_H
#define VS1053B_H

#include <types.h>

int vs1053b_init();
inline int vs1053b_check_dreq();
inline void vs1053b_wait_dreq();
inline void vs1053b_writesdi(uint8 *data, int len);
void vs1053b_cancel();
void vs1053b_set_hi_fclk();
void vs1053b_set_low_fclk();
int vs1053b_get_decode_time();
void vs1053b_set_decode_time(uint16 time);
void vs1053b_set_play_speed(uint16 value);
int vs1053b_set_volume(int value);

#endif

