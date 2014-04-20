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
#include <LPC177x_8x_bits.h>
#include <debug.h>
#include <os.h>
#include "gpioirq.h"
#include "irqp.h"
#include "buttons.h"
#include "vs1053b/vs1053b_hw.h"


struct gpio_irq_t gpioirq;

/*
 *
 */
void gpioirq_init()
{
    {

#ifdef GPIOIRQ_PORT0_FMASK
        LPC_GPIOINT->IO0IntEnF |= GPIOIRQ_PORT0_FMASK;
#endif
#ifdef GPIOIRQ_PORT2_FMASK
        LPC_GPIOINT->IO2IntEnF |= GPIOIRQ_PORT2_FMASK;
#endif
#ifdef GPIOIRQ_PORT0_RMASK
        LPC_GPIOINT->IO0IntEnR |= GPIOIRQ_PORT0_RMASK;
#endif
#ifdef GPIOIRQ_PORT2_RMASK
        LPC_GPIOINT->IO2IntEnR |= GPIOIRQ_PORT2_RMASK;
#endif
    }

    NVIC_SetPriority(GPIO_IRQn, GPIO_IRQP);
    NVIC_EnableIRQ(GPIO_IRQn);
}

/*
 *
 */
void GPIO_Handler(void)
{
//    dprint("sn", "Handler");
    /*
     * Buttons
     */
    {
        int bpressed, enc;

        bpressed = 0;
        enc      = 0;

        /*
         * Buttons interrupts
         */
#ifdef BUTTON_PORT0_INTMASK
        if (LPC_GPIOINT->IO0IntStatF & BUTTON_PORT0_INTMASK)
            bpressed = 1;
#endif
#ifdef BUTTON_PORT2_INTMASK
        if (LPC_GPIOINT->IO2IntStatF & BUTTON_PORT2_INTMASK)
            bpressed = 1;
#endif
        if (bpressed)
        {
#ifdef BUTTON_PORT0_INTMASK
            LPC_GPIOINT->IO0IntClr = BUTTON_PORT0_INTMASK;
            LPC_GPIOINT->IO0IntEnF &= ~BUTTON_PORT0_INTMASK;
#endif
#ifdef BUTTON_PORT2_INTMASK
            LPC_GPIOINT->IO2IntClr = BUTTON_PORT2_INTMASK;
            LPC_GPIOINT->IO2IntEnF &= ~BUTTON_PORT2_INTMASK;
#endif
            os_event_raise(&gpioirq.evirq, GPIOIRQ_EVENT_BUTTON);
        }

        /*
         * Encoder interrupts
         */
#ifdef ENCODER_PORT0_INTMASK
        if (LPC_GPIOINT->IO0IntStatR & ENCODER_PORT0_INTMASK)
        {
            LPC_GPIOINT->IO0IntClr  = ENCODER_PORT0_INTMASK;
            LPC_GPIOINT->IO2IntEnR &= ~ENCODER_PORT0_INTMASK;
            enc = 1;
        }
#endif

        if (enc)
            os_event_raise(&gpioirq.evirq, GPIOIRQ_EVENT_ENCODER);
    }
}

