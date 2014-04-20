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
 */
#ifndef VERSION_H
#define VERSION_H

#define MODULE_NAME "SK-MLPC1788"

#define MAJOR  0
#define MINOR  1
#define BUILD  1

#define QUOTEME_(x) #x
#define QUOTEME(x) QUOTEME_(x)

#define VERSION_STRING QUOTEME(MAJOR.MINOR.BUILD)

#endif

/*
 * CHANGELOG
 *
 *   20130620 0.1.1
 *      * initial version
 */

