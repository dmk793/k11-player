/*
 * NOTE TIM0 is used, NOT Cortext M3 system tick timer
 */
#include <LPC177x_8x.h>
#include <LPC177x_8x_bits.h>
#include "clk_cfg.h"
#include "stimer.h"

/*
 * initialize TIMER0
 */
void stimer_init()
{
    /*
     * setup TIM0 as 1ms interval timer
     */
    LPC_SC->PCONP |= PCONP_PCTIM0;

    LPC_TIM0->TCR = TCR_RESET;
    LPC_TIM0->CTCR = 0; /* timer mode */

    /*
     * for 1ms interval
     *
     * PR = 1e-3/(1/PCLK)
     *
     */
    LPC_TIM0->PR  = CLK_PCLK/1000 - 1;
    LPC_TIM0->TCR = TCR_ENABLE;

    /*
     * setup TIM2 as 1us interval timer
     */
    LPC_SC->PCONP |= PCONP_PCTIM2;

    LPC_TIM2->TCR = TCR_RESET;
    LPC_TIM2->CTCR = 0; /* timer mode */

    /*
     * for 1us interval
     *
     * PR = 1e-6/(1/(PCLK))
     *
     */
    LPC_TIM2->PR = CLK_PCLK/1000000 - 1;
    LPC_TIM2->TCR = TCR_ENABLE;
}

/*
 * initialize time value with jiffies
 */
void stimer_settime(uint32 *val)
{
    *val = LPC_TIM0->TC;
}

/*
 * return difference between current TIMER0 value and old time value
 */
uint32 stimer_deltatime(uint32 time)
{
    uint32 j;

    j = LPC_TIM0->TC;

    return (j >= time ? (j - time) : (0xffffffff - time + j));
}

/*
 * wait for specific time
 *
 * NOTE watchdog timeout can occure
 */
void stimer_wait(uint32 time)
{
    uint32 t;

    stimer_settime(&t);
    while (stimer_deltatime(t) < time)
        ;
}

/*
 * initialize time value with jiffies
 */
void stimer_settime_us(uint32 *val)
{
    *val = LPC_TIM2->TC;
}

/*
 * return difference between current TIMER0 value and old time value
 */
uint32 stimer_deltatime_us(uint32 time)
{
    uint32 j;

    j = LPC_TIM2->TC;

    return (j >= time ? (j - time) : (0xffffffff - time + j));
}

/*
 * wait for specific time
 *
 * NOTE watchdog timeout can occure
 */
void stimer_wait_us(uint32 time)
{
    uint32 t;

    stimer_settime_us(&t);
    while (stimer_deltatime_us(t) < time)
        ;
}

