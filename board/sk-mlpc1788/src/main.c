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

/*
 *
 */
#include <debug.h>
#include <stimer.h>
#include <uart.h>
#include <stimer.h>
#include <hwcrc.h>
#include <gpio.h>
#include <os.h>
#include "osw_objects.h"
#include "version.h"

int putChar(int c);

/*
 *
 */
int main(void)
{
    uart_setup(DEBUG_UART, UART_BAUD_115200, NULL);

    stimer_init();
    gpio_init();

    dprint("nsn", "=================================");
    dprint("sn",  "= "MODULE_NAME);
    dprint("sn",  "=");
    dprint("sn",  "= v"VERSION_STRING", "__DATE__" "__TIME__);
    dprint("sn",  "=================================");

    os_init();
    osw_init();
    os_start();

    for (;;);

    return 0;
}

