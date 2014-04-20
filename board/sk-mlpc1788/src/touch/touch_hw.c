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
 * touch screen hardware functions
 *
 * TSC2046 touch screen controller
 */
#include <LPC177x_8x.h>
#include <LPC177x_8x_bits.h>
#include <gpio.h>
#include <clk_cfg.h>
#include <debug.h>
#include <stimer.h>
#include "touch_hw.h"


static const struct gpio_t ts_cs   = {LPC_GPIO2, (1 << 11)};
static const struct gpio_t ts_clk  = {LPC_GPIO0, (1 << 18)};
static const struct gpio_t ts_dout = {LPC_GPIO0, (1 << 17)};
static const struct gpio_t ts_din  = {LPC_GPIO0, (1 << 20)};

BASE_TYPE touch_event;
static uint32 timeout;

static void touch_hw_spi_exec(uint8 *in, uint8 *out, int cnt);

#define DELAY2TICK(d) ((uint32)(1ULL * d * CLK_CCLK / 1000000000ULL))
//#define DELAY_NS(d)   {touch_hw_sdelay(DELAY2TICK(d));}
#define DELAY_NS(d)   {asm volatile("nop\n");}
static void touch_hw_sdelay(int delay);

/*
 *
 */
void touch_hw_init()
{
    /*
     * TODO connect to hardware SPI
     *
     * configure pins
     *
     * Function          Port    MCU Pin   Net Name    Target function
     * ---------------------------------------------------------------
     * GPIO              P2[11]  108       LCD27       CS
     * GPIO              P0[18]  124       MP124       DCLK
     * GPIO              P0[17]  126       MP126       DOUT
     * GPIO              P0[20]  120       MP120       DIN
     * GPIO              P0[19]  122       PRG         PENIRQ     
     */
    gpio_setdir(&ts_cs,  GPIO_DIR_OUTPUT);
    gpio_setdir(&ts_clk, GPIO_DIR_OUTPUT);
    gpio_setdir(&ts_din, GPIO_DIR_OUTPUT);

//    /* disable pull resistors for PENIRQ */
//    LPC_IOCON_MODE_NO_PULLUP_NO_PULLDOWN(P0_19);

    gpio_drive(&ts_cs, 1);
    gpio_drive(&ts_clk, 0);
    gpio_drive(&ts_din, 0);

    stimer_settime(&timeout);

    /* configure interrupt for PENIRQ */
#define PENIRQ_PORT_MASK    (1 << 19)
    LPC_GPIOINT->IO0IntEnF |= PENIRQ_PORT_MASK; /* enable falling edge interrupt on PENIRQ */
    NVIC_EnableIRQ(GPIO_IRQn);
}

/*
 *
 */
void GPIO_Handler()
{
    /* XXX process only P0.19 interrupt */
    if (LPC_GPIOINT->IO0IntStatF & PENIRQ_PORT_MASK)
    {
        LPC_GPIOINT->IO0IntClr  =  PENIRQ_PORT_MASK;

#define MESURE_TIMEOUT    200 
        if (stimer_deltatime(timeout) >= MESURE_TIMEOUT)
        {
            stimer_settime(&timeout);

            LPC_GPIOINT->IO0IntEnF &= ~PENIRQ_PORT_MASK;
            NVIC_DisableIRQ(GPIO_IRQn);

            os_event_raise(&touch_event, TOUCH_EVENT_MASK_IRQ);
        }
    }
}

/*
 * ARGS
 *     a   value of A0, A1, A2
 *
 * RETURN
 *     result of 
 */
#define TCONTROL_SER           (1 << 2)
#define TCONTROL_DFR           (0 << 2)
#define TCONTROL_MODE_12BIT    (0 << 3)
#define TCONTROL_MODE_8BIT     (1 << 3)
#define TCONTROL_CHSEL_SINGLE_Y_MESURE    (1 << 4)
#define TCONTROL_CHSEL_SINGLE_X_MESURE    (5 << 4)
#define TCONTROL_CHSEL_DIF_Y_MESURE       (1 << 4)
#define TCONTROL_CHSEL_DIF_X_MESURE       (5 << 4)
#define TCONTROL_CHSEL_DIF_Z1_MESURE      (3 << 4)
#define TCONTROL_CHSEL_DIF_Z2_MESURE      (4 << 4)
#define TOUCH_HW_GETX() touch_hw_getval(TCONTROL_CHSEL_SINGLE_X_MESURE)
#define TOUCH_HW_GETY() touch_hw_getval(TCONTROL_CHSEL_SINGLE_Y_MESURE)
static uint16 touch_hw_getadc(uint8 a, uint8 pd)
{
    uint8 in[3];
    uint8 out[3];

    in[0] = (pd << 0) | TCONTROL_SER | TCONTROL_MODE_12BIT | (1 << 7) | a;
    in[1] = 0;
    in[2] = 0;
    touch_hw_spi_exec(in, out, 3);
    return (((out[1] & 0x7f) << 5) | (out[2] >> 3));
}


static uint16 touch_hw_getsample(uint8 a, uint8 pd, int nsample)
{
    int i, j;
    uint16 tval;
#define SAMPLE_COUNT_MAX   9
    uint16 sample[SAMPLE_COUNT_MAX];
    uint16 result;

    if (nsample > SAMPLE_COUNT_MAX)
        nsample = SAMPLE_COUNT_MAX;

    for (i = 0; i < nsample; i++)
        sample[i] = touch_hw_getadc(a, pd);

    if (nsample > 2)
    {
        /* sort samples */
        for (i = 1; i < nsample; i++)
        {
            tval = sample[i];
            j = i - 1;

            while (j >= 0 && sample[j] > tval)
            {
                sample[j + 1] = sample[j];
                j--;
            }
            sample[j + 1] = tval;
        }

        /* drop first and last sample, get average value */
        result = 0;
        for (i = 1; i < nsample - 1; i++)
            result += sample[i];

        return result / (nsample - 2);
    } else {
        result = 0;
        for (i = 0; i < nsample; i++)
            result += sample[i];

        return result / nsample;
    }
}

/*
 * ARGS
 *     swidth    horizontal resolution of screen
 *     sheight   vertical resolution of screen
 *
 * RETURN
 *     1 if valid coordianates available
 */
#define X0_ADC     3850
#define X1_ADC     230
#define Y0_ADC     3620
#define Y1_ADC     330
#define Z1_MIN_ADC 50

int touch_hw_getvalues(int swidth, int sheight, uint16 *x, uint16 *y)
{
    uint16 z1;
    int ret;

    ret = 0;
    gpio_drive(&ts_cs, 0);
    {
        /* Tcss >= 100 ns */
        DELAY_NS(100);

        z1 = touch_hw_getsample(TCONTROL_CHSEL_DIF_Z1_MESURE, 1, 3);
        /* XXX */
        if (z1 > Z1_MIN_ADC)
        {
            *x = touch_hw_getsample(TCONTROL_CHSEL_DIF_X_MESURE, 1, 7);
            *y = touch_hw_getsample(TCONTROL_CHSEL_DIF_Y_MESURE, 1, 7);

            if (*x > X0_ADC) *x = X0_ADC;
            if (*x < X1_ADC) *x = X1_ADC;
            if (*y > Y0_ADC) *y = Y0_ADC;
            if (*y < Y1_ADC) *y = Y1_ADC;

            *x = swidth  - swidth  * (*x - X1_ADC) / (X0_ADC - X1_ADC);
            *y = sheight - sheight * (*y - Y1_ADC) / (Y0_ADC - Y1_ADC);

            ret = 1;
        }
        /* XXX dummy read */
        z1 = touch_hw_getsample(TCONTROL_CHSEL_DIF_Z1_MESURE, 0, 1);
    }
    gpio_drive(&ts_cs, 1);

    LPC_GPIOINT->IO0IntClr  =  PENIRQ_PORT_MASK;
    LPC_GPIOINT->IO0IntEnF |= PENIRQ_PORT_MASK;
    NVIC_EnableIRQ(GPIO_IRQn);

    return ret;
}



/*
 *
 */
static void touch_hw_spi_exec(uint8 *in, uint8 *out, int cnt)
{
    uint8 mask;

    /*
     * MSB first
     * MOSI sampled by slave at rising edge
     * MISO sampled by master at rising edge
     *
     * DCLK low  >= 200ns
     * DCLK high >= 200ns
     */
    mask = 0x80;
    *out = 0;
    gpio_drive(&ts_clk, 0);
    while (cnt)
    {
        if (*in & mask)
            gpio_drive(&ts_din, 1);
        else
            gpio_drive(&ts_din, 0);

        DELAY_NS(100);
        gpio_drive(&ts_clk, 1);
        if (gpio_read(&ts_dout))
            *out |= mask;
        DELAY_NS(100);

        gpio_drive(&ts_clk, 0);

        mask >>= 1;
        if (mask == 0x00)
        {
            mask = 0x80;
            in++;
            out++;
            cnt--;
            if (cnt)
                *out = 0;
        }
    }
}

/*
 *
 */
static void __attribute__((optimize("O0"))) touch_hw_sdelay(int delay)
{
    while (delay--)
        asm volatile("nop");
}

