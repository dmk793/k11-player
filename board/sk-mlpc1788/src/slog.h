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

#ifndef SLOG_H
#define SLOG_H

#include <os.h>
#include <types.h>

int slog_append(int ch);
int slog_lines();
char* slog_get_line(char *st, int maxsize, int n);

void slog_lock();
void slog_unlock();

#define SLOG_EVENT_MASK_UPDATE   (1 << 0)
extern BASE_TYPE slog_event;

#endif

