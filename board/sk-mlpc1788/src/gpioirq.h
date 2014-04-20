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

#ifndef GPIOIRQ_H
#define GPIOIRQ_H

#include <types.h>
#include <os.h>

struct gpio_irq_t {
#define GPIOIRQ_EVENT_BUTTON   (1 << 0)
#define GPIOIRQ_EVENT_ENCODER  (1 << 1)
    BASE_TYPE evirq;
};

extern struct gpio_irq_t gpioirq;

void gpioirq_init();

#endif

