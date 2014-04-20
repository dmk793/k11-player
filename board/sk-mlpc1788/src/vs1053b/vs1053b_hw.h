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

#ifndef VS1053B_HW_H
#define VS1053B_HW_H

#include <types.h>

int vs1053b_hw_init();
void inline vs1053b_hw_reset_on();
void inline vs1053b_hw_reset_off();
uint16 vs1053b_hw_readsci(uint8 addr);
void vs1053b_hw_writesci(uint8 addr, uint16 value);
void vs1053b_hw_writesdi(uint8 *data, int len);
int vs1053b_hw_check_dreq();
void vs1053b_hw_wait_dreq();
void inline vs1053b_hw_set_hi_fclk();
void inline vs1053b_hw_set_low_fclk();

#endif

