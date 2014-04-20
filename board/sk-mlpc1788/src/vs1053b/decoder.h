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

#ifndef DECODER_H
#define DECODER_H

#include <types.h>

//#define DEC_TEST_GPIO

#ifdef DEC_TEST_GPIO
    #include <LPC177x_8x.h>
    #include <gpio.h>
#endif

#ifdef DEC_TEST_GPIO
extern const struct gpio_t dec_testpins[2];

#define DECTEST_GPIO0  (&dec_testpins[0])
#define DECTEST_GPIO1 (&dec_testpins[1])
#endif

void decoder_task();
int decoder_set_volume(int value);

#endif

