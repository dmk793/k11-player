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

#include <LPC177x_8x.h>
#include <stimer.h>
#include <debug.h>
#include <os.h>
#include <gpio.h>
#include "buttons.h"
#include "irqp.h"
#include "player/player.h"
#include "gpioirq.h"

//#define DEBUG_KEY

#ifdef DEBUG_KEY
    #define DPRINT(fmt, ...)  dprint(fmt, __VA_ARGS__)
#else
    #define DPRINT(fmt, ...)
#endif

/* 
 *
 * KEY_1    MP64    P2.23
 * KEY_2    MP67    P2.19
 * KEY_3    MP54    P2.25
 * KEY_4    MP91    P2.14 *
 * KEY_5    MP81    P2.21 *
 * KEY_6    MP85    P2.22 *
 *
 */
#define BUTTON_COUNT    6
const struct gpio_t gbutton[BUTTON_COUNT] = {
    {LPC_GPIO2, (1 << 23)},
    {LPC_GPIO2, (1 << 19)},
    {LPC_GPIO2, (1 << 25)},
    {LPC_GPIO2, (1 << 14)},
    {LPC_GPIO2, (1 << 21)},
    {LPC_GPIO2, (1 << 22)},
};
#define GBUTTON_UP     (&gbutton[5]) /* 0 */
#define GBUTTON_DOWN   (&gbutton[3]) /* 1 */
#define GBUTTON_PGUP   (&gbutton[0]) /* 2 */
#define GBUTTON_PGDOWN (&gbutton[2]) /* 3 */
#define GBUTTON_ENTER  (&gbutton[1]) /* 4 */
#define GBUTTON_BACK   (&gbutton[4]) /* 5 */

#define BPRESSED(b) (gpio_read((b)->pg) == 0)

const struct gpio_t encpin[2] = {
    {LPC_GPIO0, (1 << 26)}, /* MP12,  P0.26 */
    {LPC_GPIO3, (1 << 27)}, /* MP203, P3.27 */
};
#define ENC_CNT_PIN    (&encpin[0])
#define ENC_DIR_PIN    (&encpin[1])


struct button_t {
    uint32 code;
#define BUTTON_CHECK_TO     20
#define BUTTON_CHECK_COUNT  5
#define BUTTON_REPEAT_TO    200
    int state;
    const struct gpio_t *pg;
};

static struct button_t button[BUTTON_COUNT];

static void check_buttons();
static void check_encoder();

/*
 *
 */
void buttons_task()
{
    button[0].pg = GBUTTON_UP    ;
    button[1].pg = GBUTTON_DOWN  ;
    button[2].pg = GBUTTON_PGUP  ;
    button[3].pg = GBUTTON_PGDOWN;
    button[4].pg = GBUTTON_ENTER ;
    button[5].pg = GBUTTON_BACK  ;

    button[0].code = BUTTON_UP;
    button[1].code = BUTTON_DOWN;
    button[2].code = BUTTON_PGUP;
    button[3].code = BUTTON_PGDOWN;
    button[4].code = BUTTON_ENTER;
    button[5].code = BUTTON_BACK;

    /* take some pause for initialization of player message queue */
    os_event_wait(&player.event, PLAYER_EVENT_INIT, OS_FLAG_NONE, OS_WAIT_FOREVER);

    while (1)
    {
        os_event_wait(&gpioirq.evirq, GPIOIRQ_EVENT_BUTTON | GPIOIRQ_EVENT_ENCODER, OS_FLAG_NONE, OS_WAIT_FOREVER);

        if (os_event_wait(&gpioirq.evirq, GPIOIRQ_EVENT_BUTTON,
                    OS_FLAG_NOWAIT | OS_FLAG_CLEAR, OS_WAIT_FOREVER) == OS_ERR_NONE)
        {
            check_buttons();
        } else if (os_event_wait(&gpioirq.evirq, GPIOIRQ_EVENT_ENCODER,
                    OS_FLAG_NOWAIT | OS_FLAG_CLEAR, OS_WAIT_FOREVER) == OS_ERR_NONE) {
            check_encoder();
        }
    }
}

/*
 *
 */
static void
check_buttons()
{
    int i, n, fpress;
    uint32 code;
    struct player_msg_t msg;
    uint32 timeout;

    fpress = 1;
    do {
        code = 0;
        for (i = 0; i < BUTTON_COUNT; i++)
            button[i].state = 0;

        n = BUTTON_CHECK_COUNT;
        while (n--)
        {
            os_wait_ms(BUTTON_CHECK_TO);
            for (i = 0; i < BUTTON_COUNT; i++)
                if (BPRESSED(&button[i]))
                    button[i].state++;
        }

        for (i = 0; i < BUTTON_COUNT; i++)
            if (button[i].state > (BUTTON_CHECK_COUNT / 2))
                code |= button[i].code;

        if (code)
        {
            if (fpress || stimer_deltatime(timeout) >= BUTTON_REPEAT_TO)
            {
                stimer_settime(&timeout);
                fpress = 0;
#ifdef DEBUG_KEY
                DPRINT("s_", "pressed:");
                if (code & BUTTON_UP    ) DPRINT("s_", "BUTTON_UP");
                if (code & BUTTON_DOWN  ) DPRINT("s_", "BUTTON_DOWN");
                if (code & BUTTON_PGUP  ) DPRINT("s_", "BUTTON_PGUP");
                if (code & BUTTON_PGDOWN) DPRINT("s_", "BUTTON_PGDOWN");
                if (code & BUTTON_ENTER ) DPRINT("s_", "BUTTON_ENTER");
                if (code & BUTTON_BACK  ) DPRINT("s_", "BUTTON_BACK");
                DPRINT("sn","");
#endif
                msg.id = PLAYER_MSG_ID_BUTTON;
                msg.button.code = code;
                PLAYER_SEND_MSG(&msg, pmsg_button_t);
            }
        }
    } while (code);

#ifdef BUTTON_PORT0_INTMASK
    LPC_GPIOINT->IO0IntClr  = BUTTON_PORT0_INTMASK;
    LPC_GPIOINT->IO0IntEnF |= BUTTON_PORT0_INTMASK;
#endif
#ifdef BUTTON_PORT2_INTMASK
    LPC_GPIOINT->IO2IntClr  = BUTTON_PORT2_INTMASK;
    LPC_GPIOINT->IO2IntEnF |= BUTTON_PORT2_INTMASK;
#endif
}


/*
 *
 */
static void
check_encoder()
{
    struct player_msg_t msg;
    int cntsample;
    int dirsample;
    int n;

#define SAMPLE_COUNT 16
    cntsample = 0;
    dirsample = 0;
    os_disable_irq();
    {
//        for (n = 0; n < SAMPLE_COUNT; n++)
//            asm volatile("nop");

        for (n = 0; n < SAMPLE_COUNT; n++)
        {
            if (gpio_read(ENC_DIR_PIN))
                dirsample++;
            asm volatile("nop");
        }

        for (n = 0; n < SAMPLE_COUNT; n++)
        {
            asm volatile("nop");
            if (gpio_read(ENC_CNT_PIN))
                cntsample++;
        }
    }
    os_enable_irq();
    if (cntsample == n)
    {
        msg.id = PLAYER_MSG_ID_BUTTON;
        if (dirsample < (SAMPLE_COUNT / 2))
            msg.button.code = ENCODER_PLUS;
        else
            msg.button.code = ENCODER_MINUS;
        PLAYER_SEND_MSG(&msg, pmsg_button_t);
    }

    /* NOTE some delay */
    os_wait_ms(20);

#ifdef ENCODER_PORT0_INTMASK
    LPC_GPIOINT->IO0IntClr  = ENCODER_PORT0_INTMASK;
    LPC_GPIOINT->IO0IntEnR |= ENCODER_PORT0_INTMASK;
#endif
}

