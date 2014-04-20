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

#ifndef BUTTONS_H
#define BUTTONS_H

#include <types.h>

void buttons_task();

#define BUTTON_UP      (1 << 0)
#define BUTTON_DOWN    (1 << 1)
#define BUTTON_PGUP    (1 << 2)
#define BUTTON_PGDOWN  (1 << 3)
#define BUTTON_ENTER   (1 << 4)
#define BUTTON_BACK    (1 << 5)
#define ENCODER_PLUS   (1 << 6)
#define ENCODER_MINUS  (1 << 7)

#define BUTTON_PRESSED(bcode, mask) ((bcode & (mask)) == (mask))

#define ENCODER_PORT0_INTMASK ((1 << 26))
#define BUTTON_PORT2_INTMASK  ((1 << 23) | (1 << 19) | (1 << 25) | (1 << 14) | (1 << 21) | (1 << 22) )

#define GPIOIRQ_PORT0_RMASK (ENCODER_PORT0_INTMASK)
#define GPIOIRQ_PORT2_FMASK (BUTTON_PORT2_INTMASK)


#endif

