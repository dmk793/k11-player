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
 * OS objects definitions, wrappers, startup routines.
 */
#include <string.h>
#include <os.h>
#include <os_private.h>
#include <port/ARMv7-M/port.h>
#include <debug.h>
#include "osw_objects.h"
#include "net/net.h"
#include "gs/gs.h"
#include "testgs.h"
#include "slog.h"
#include "gui/gslog.h"
#include "upload.h"
#include "usbdev/usbdev.h"
#include "gtask.h"
#include "dma.h"
#include "player/player.h"
#include "sdcard/sdcard.h"
#include "buttons.h"
#include "vs1053b/decoder.h"
#include "gpioirq.h"

#define DEFAULT_STACK_SIZE (32 * 1024)

struct osw_task_info_t {
    char name[16];
    char enabled;       /* 1 - initialize task, 0 - not initialize */
    BASE_TYPE ssize;    /* size of stack */
    BASE_TYPE priority; /* NOTE not implemented */
    void (*entry)();
    void *context;
};

static const struct osw_task_info_t tinfo[OS_CONFIG_TASK_COUNT] = {
    /* task's name    enabled    stack size            priority,     entry point,       context */
    {"General  ",      0,         DEFAULT_STACK_SIZE,          0,    gtask,             NULL},
    {"Net      ",      0,         DEFAULT_STACK_SIZE,          0,    net_task,          NULL},
    {"Net Proc ",      0,         DEFAULT_STACK_SIZE,          1,    net_process_task,  NULL},
    {"GS       ",      1,         DEFAULT_STACK_SIZE,          3,    gs_task,           NULL},
    {"GS Test  ",      0,         DEFAULT_STACK_SIZE,          3,    testgs_task,       NULL},
    {"USB Dev  ",      0,         DEFAULT_STACK_SIZE,          0,    usbdev_task,       NULL},
    {"Player   ",      1,         DEFAULT_STACK_SIZE,          1,    player_task,       NULL},
    {"Decoder  ",      1,         DEFAULT_STACK_SIZE,          0,    decoder_task,      NULL},
    {"Buttons  ",      1,         DEFAULT_STACK_SIZE,        255,    buttons_task,      NULL},
    {"         ",      0,         DEFAULT_STACK_SIZE,          0,    NULL,              NULL},
    {"         ",      0,         DEFAULT_STACK_SIZE,          0,    NULL,              NULL},
    {"         ",      0,         DEFAULT_STACK_SIZE,          0,    NULL,              NULL},
    {"         ",      0,         DEFAULT_STACK_SIZE,          0,    NULL,              NULL},
    {"         ",      0,         DEFAULT_STACK_SIZE,          0,    NULL,              NULL},
    {"         ",      0,         DEFAULT_STACK_SIZE,          0,    NULL,              NULL},
    {"         ",      0,         DEFAULT_STACK_SIZE,          0,    NULL,              NULL},
};

#define OSW_STACK_SPOOL_SIZE    (DEFAULT_STACK_SIZE * OS_CONFIG_TASK_COUNT)
uint8 sspool[OSW_STACK_SPOOL_SIZE] __attribute__((section("tstack")));


static void osw_print_tasks_state();
static void osw_void_handler();

extern uint32 *_uvect_start;
extern uint32 *_uvect_size;

void * userfunctions[] = {
    &osw_print_tasks_state, /* 0 */
    &osw_void_handler,      /* 1 */
    &osw_void_handler,      /* 2 */
    &osw_void_handler,      /* 3 */
};

/*
 * Initialize tasks, queues and other OS related structures.
 */
void osw_init()
{
    int i;
    BASE_TYPE sstart;

//    LPC_GPIO0->DIR = (1 << 11);
//    LPC_GPIO0->CLR = (1 << 11);
//
//    /* add system log to debug output */
//    debug_add_cb(slog_append);

    /* Copy user-defined table of functions */
    {
        void *table;
        uint32 size;

        table =         &_uvect_start;
        size  = (uint32)&_uvect_size;

        memcpy(table, userfunctions, size);
    }

    /*
     * Initialize tasks
     */
    dprint("sn", "Init tasks:");
    sstart = 0;
    for (i = 0; i < OS_CONFIG_TASK_COUNT; i++)
    {
        struct osw_task_info_t *ti;

        ti = (struct osw_task_info_t *)&tinfo[i];
        if ((sstart + ti->ssize) > OSW_STACK_SPOOL_SIZE)
        {
            dprint("sn", ERR_PREFIX "stack spool overflow");
            break;
        }

        /* NOTE for debug purposes fill stack with initial values */
        memset(&sspool[sstart], 0xAB, ti->ssize);

        {
            BASE_TYPE saddr_start;
            BASE_TYPE saddr_end;
            BASE_TYPE entry;

            saddr_start = (BASE_TYPE)sspool + sstart;
            saddr_end   = saddr_start + ti->ssize - 1;
            entry       = (BASE_TYPE)ti->entry;

            dprint("tg1d_ c_s 4x*:4xs",
                    2, i, 
                    ti->enabled ? '+' : '-',
                    ti->name,
                    saddr_start,
                    saddr_end, ", ");
            if (ti->entry)
                dprint("4xs", entry, "\r\n");
            else
                dprint("s", "NULL\r\n");
        }

        if (ti->enabled && ti->entry != NULL)
            OS_TASK_INIT(ti->name, &sspool[sstart], ti->ssize, ti->priority, ti->entry, ti->context);

        sstart += ti->ssize;
    }

    /* NOTE initialize DMA */
    dma_init();

    gpioirq_init();
}

/*
 * print tasks state
 */
static void osw_print_lock_state(struct os_task_lock_t *lock);

static void osw_print_tasks_state()
{
    int i;
    volatile struct os_taskcb_t *ptcb;
    struct os_task_context_t    *tcont;

    ptcb = os_tasks;
    for (i = 0; i < os_taskidx; i++)
    {
#ifdef OS_CONFIG_TASK_NAME_SIZE
        dprint("<Task #>4d_*(s*)sn", i, ptcb->name, ptcb == os_current_taskcb ? " (CURRENT)" : "");
#else
        dprint("<Task #>4dn", i);
#endif
        dprint("t<TOS   >4xn", ptcb->tos);
        tcont = (struct os_task_context_t *)ptcb->tos;
        dprint("tsn", "-context-");
        dprint("tt<r0    >4xn", tcont->r0);
        dprint("tt<r1    >4xn", tcont->r1);
        dprint("tt<r2    >4xn", tcont->r2);
        dprint("tt<r3    >4xn", tcont->r3);
        dprint("tt<r4    >4xn", tcont->r4);
        dprint("tt<r5    >4xn", tcont->r5);
        dprint("tt<r6    >4xn", tcont->r6);
        dprint("tt<r7    >4xn", tcont->r7);
        dprint("tt<r8    >4xn", tcont->r8);
        dprint("tt<r9    >4xn", tcont->r9);
        dprint("tt<r10   >4xn", tcont->r10);
        dprint("tt<r11   >4xn", tcont->r11);
        dprint("tt<r12   >4xn", tcont->r12);
        dprint("tt<lr    >4xn", tcont->lr);
        dprint("tt<pc    >4xn", tcont->pc);
        dprint("tt<xpsr  >4xn", tcont->xpsr);
        if (ptcb->lock.state & OS_TASK_LOCKED)
            osw_print_lock_state((struct os_task_lock_t *)&ptcb->lock);
        dprint("tsn", "---");
#ifdef OS_USE_TIMEOUT
        dprint("tt<timeout  >4xsn", ptcb->timeout, ptcb->timeout ? "" : " (FOREVER)");
#endif
#ifdef OS_CONFIG_USE_TASK_SLICE
        dprint("tt<tslice   >4xn", ptcb->tslice);
#ifdef OS_CONFIG_USE_VARIABLE_TASK_SLICE
        dprint("tt<dslice   >4xn", ptcb->dslice);
#endif
#endif
        ptcb++;
    }
}

static void osw_print_lock_state(struct os_task_lock_t *lock)
{
    char *lname;
    dprint("tsn", "-lock-");

    switch (lock->state)
    {
        case OS_TASK_STATE_RUN               : lname = "RUN";                break;
        case OS_TASK_STATE_SUSPEND           : lname = "SUSPEND";            break;
#if OS_USE_LOCK
        case OS_TASK_STATE_LOCKED_WAIT       : lname = "LOCKED WAIT";        break;
        case OS_TASK_STATE_LOCKED_MUTEX      : lname = "LOCKED MUTEX";       break;
        case OS_TASK_STATE_LOCKED_EVENT      : lname = "LOCKED EVENT";       break;
        case OS_TASK_STATE_LOCKED_QUEUE_FULL : lname = "LOCKED QUEUE FULL";  break;
        case OS_TASK_STATE_LOCKED_QUEUE_EMPTY: lname = "LOCKED QUEUE EMPTY"; break;
        case OS_TASK_STATE_LOCKED_MULTI      : lname = "LOCKED MULTI";       break;
#endif
        default:
            lname = "";
    }
    dprint("tt<state    >4x_*(s*)n", lock->state, lname);

    switch (lock->state)
    {
#if OS_USE_LOCK
        case OS_TASK_STATE_LOCKED_WAIT:
            break;
#ifdef OS_CONFIG_USE_MUTEX
        case OS_TASK_STATE_LOCKED_MUTEX:
            dprint("tt<pobj     >4xn",  lock->pobj);
            dprint("tt<mask     >4xn",  lock->mask);
            break;
#endif
#ifdef OS_CONFIG_USE_EVENT
        case OS_TASK_STATE_LOCKED_EVENT:
            dprint("tt<pobj     >4xn",  lock->pobj);
            dprint("tt<mask     >4xn",  lock->mask);
            break;
#endif
#ifdef OS_CONFIG_USE_QUEUE
        case OS_TASK_STATE_LOCKED_QUEUE_FULL:
            {
                struct os_queue_t *q;
                q = (struct os_queue_t*)lock->pobj;

                dprint("tt<pobj     >4xn",  lock->pobj);
                dprint("ttsn","-queue-");
                dprint("ttt<mutex     >4xn",  q->mutex);
                dprint("ttt<qsize     >4xn",  q->qsize);
                dprint("ttt<msize     >4xn",  q->msize);
                dprint("ttt<count     >4xn",  q->count);
                dprint("ttt<wp        >4xn",  q->wp);
                dprint("ttt<spool     >4xn",  q->spool);
            }
            break;
        case OS_TASK_STATE_LOCKED_QUEUE_EMPTY:
            {
                struct os_queue_t *q;
                q = (struct os_queue_t*)lock->pobj;

                dprint("tt<pobj     >4xn",  lock->pobj);
                dprint("ttsn","-queue-");
                dprint("ttt<mutex     >4xn",  q->mutex);
                dprint("ttt<qsize     >4xn",  q->qsize);
                dprint("ttt<msize     >4xn",  q->msize);
                dprint("ttt<count     >4xn",  q->count);
                dprint("ttt<wp        >4xn",  q->wp);
                dprint("ttt<spool     >4xn",  q->spool);
            }
            break;
#endif
#ifdef OS_CONFIG_USE_MULTI
        case OS_TASK_STATE_LOCKED_MULTI:
            {
                struct os_multi_event_t *multi;
                struct os_task_lock_t *event;
                int n;

                multi = (struct os_multi_event_t*)lock->pobj;

                dprint("tt<pobj     >4xn",  lock->pobj);
                dprint("ttsn","-multi-");
                dprint("ttt<count     >4xn",  multi->count);
                dprint("ttt<n         >4xn",  multi->n);
                dprint("ttt<type      >4xn",  multi->type);
                dprint("ttt<events    >4xn",  multi->events);
                dprint("ttsn","---");
                n     = multi->n;
                event = multi->events;
                while (n--)
                {
                    /* NOTE call recursively */
                    osw_print_lock_state(event++);
                }
            }
            break;
#endif
#endif /* OS_USE_LOCK */
    }
}

/*
 *
 */
static void osw_void_handler()
{


}

