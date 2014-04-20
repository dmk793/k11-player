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
---------------------------------------------------------------------------
Copyright (c) 2003, Dominik Reichl <dominik.reichl@t-online.de>, Germany.
All rights reserved.

Distributed under the terms of the GNU General Public License v2.

This software is provided 'as is' with no explicit or implied warranties
in respect of its properties, including, but not limited to, correctness 
and/or fitness for purpose.
---------------------------------------------------------------------------
*/

#ifndef ___CRC16_H___
#define ___CRC16_H___

void crc16_init(unsigned short *uCrc16);
void crc16_update(unsigned short *uCrc16, unsigned char *pBuffer, unsigned long uBufSize);
void crc16_final(unsigned short *uCrc16);

void crc16ccitt_init(unsigned short *uCcitt16);
void crc16ccitt_update(unsigned short *uCcitt16, unsigned char *pBuffer, unsigned long uBufSize);
void crc16ccitt_final(unsigned short *uCcitt16);

#endif // ___CRC16_H___

