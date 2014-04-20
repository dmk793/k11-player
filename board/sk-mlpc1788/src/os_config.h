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
#ifndef OS_CONFIG_SKMLPC1788_H
#define OS_CONFIG_SKMLPC1788_H

#define OS_BOARD_CONFIG /* to check that this file is properly included */

#define OS_CONFIG_TASK_COUNT      16           /* number of tasks */
#define OS_CONFIG_TICK_PERIOD     10000        /* period of system timer, us */
#define OS_CONFIG_USE_WAIT                     /* enable os_wait() functionality */

#define OS_CONFIG_TASK_NAME_SIZE  16

#define OS_CONFIG_USE_PRIORITY                 /* use priorities for tasks */

//#define OS_CONFIG_USE_TASK_SLICE                 /* task's time slice support */
//#define OS_CONFIG_DEFAULT_TASK_SLICE  (20 * 1000 /*us */ / OS_CONFIG_TICK_PERIOD)
//#define OS_CONFIG_USE_VARIABLE_TASK_SLICE        /* enable set of time slice of current task dynamicly */

#define OS_CONFIG_USE_MUTEX                    /* enable os_mutex_lock(), os_mutex_unlock() functionality */
#define OS_CONFIG_USE_EVENT                    /* enable os_event_raise(), os_event_wait() functionality */

#define OS_CONFIG_USE_QUEUE                    /* enable os_queue_ functionality */

#define OS_CONFIG_USE_DYNMEM                        /* enable dynamic memory functions */
#define OS_CONFIG_DYNMEM_SIZE  (16 * 1024 * 1024)   /* size of dynamic memory */
#define OS_CONFIG_DYNMEM_1                          /* first implementation of dynamic memory */
//#define OS_CONFIG_DYNMEM_2                          /* second implementation of dynamic memory */
//#define OS_CONFIG_DYNMEM_3                          /* third implementation of dynamic memory */

#define OS_CONFIG_USE_MULTI                    /* enable multiple events funcitons */
//
//#define OS_CONFIG_USE_TRACE

#define OS_CONFIG_TRAP_SCHEDQ          /* trap on scheduler queue errors */
#define OS_CONFIG_TRAP_BITOBJ          /* trap on bitobjects errors (mutexes, events) */
#define OS_CONFIG_TRAP_DYNMEM          /* trap on dynamic memory errors */
#define OS_CONFIG_TRAP_QUEUE           /* trap on queue errors */
#define OS_CONFIG_TRAP_MULTI           /* trap on multiple events errors */

#endif

