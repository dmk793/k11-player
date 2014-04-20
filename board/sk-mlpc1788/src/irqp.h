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

#ifndef IRQ_PRIORITY_T
#define IRQ_PRIORITY_T

/*
 * look at "lib/os/src/port/ARMv7-M/port.c" for split to group/sub priorities
 *
 * Systick has 1 as group priority. DPort has 0 as group priority.
 * Don't use this groups.
 *
 */
#define GROUP_PRIORITY(n) (n << 5) /* 8 values (3 bits) */
#define SUB_PRIORITY(n)   (n << 3) /* 4 values (2 bits) */

#define EINT0_IRQP (GROUP_PRIORITY(2) | SUB_PRIORITY(0))
#define MCI_IRQP   (GROUP_PRIORITY(2) | SUB_PRIORITY(0))
#define DMA_IRQP   (GROUP_PRIORITY(3) | SUB_PRIORITY(0))
#define ENET_IRQP  (GROUP_PRIORITY(4) | SUB_PRIORITY(0))
#define USB_IRQP   (GROUP_PRIORITY(7) | SUB_PRIORITY(2))
#define SSP1_IRQP  (GROUP_PRIORITY(7) | SUB_PRIORITY(3))
#define LCD_IRQP   (GROUP_PRIORITY(7) | SUB_PRIORITY(3))
#define GPIO_IRQP  (GROUP_PRIORITY(7) | SUB_PRIORITY(3))

#endif

