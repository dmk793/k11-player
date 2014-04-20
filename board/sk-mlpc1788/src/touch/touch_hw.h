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

#ifndef TOUCH_HW_H
#define TOUCH_HW_H

#include <types.h>
#include <os.h>

#define TOUCH_EVENT_MASK_IRQ    (1 << 0)
extern BASE_TYPE touch_event;

void touch_hw_init();

int touch_hw_getvalues(int swidth, int sheight, uint16 *x, uint16 *y);

#endif

