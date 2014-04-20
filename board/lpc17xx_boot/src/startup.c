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
#include <clk_cfg.h>

#define CLKSRCSEL_INTRC    (0x00)
#define CLKSRCSEL_MAINOSC  (0x01)

#define CCLKSEL_CCLKSEL_SYSCLK (0 << 8)
#define CCLKSEL_CCLKSEL_PLL    (1 << 8)
#define CCLKSEL_CCLKDIV(val)   val

#define PCLKSEL_PCLKDIV(val)   val

#define EMCCLKSEL_EMCDIV_CPU  (0 << 0)
#define EMCCLKSEL_EMCDIV_DIV2 (1 << 0)

#define SCS_OSCEN    (1 << 5)
#define SCS_OSCSTAT  (1 << 6)

#define PBOOST_BOOST (1 << 1)

extern unsigned long _sidata; /* start address for the initialization values of the .data section. defined in linker script */
extern unsigned long _sdata;  /* start address for the .data section. defined in linker script */
extern unsigned long _edata;  /* end address for the .data section. defined in linker script */

extern unsigned long _sbss; /* start address for the .bss section. defined in linker script */
extern unsigned long _ebss; /* end address for the .bss section. defined in linker script */

int main(void);

static void lowlevel_init();
static void pllfeed();

/*
 *
 */
void Reset_Handler()
{
    unsigned long *pulSrc, *pulDest;

    /* copy the data segment initializers from flash to SRAM */
    pulSrc = &_sidata;
    for(pulDest = &_sdata; pulDest < &_edata; )
    {
        *(pulDest++) = *(pulSrc++);
    }

    /* zero fill the bss segment */
    for(pulDest = &_sbss; pulDest < &_ebss; )
    {
        *(pulDest++) = 0;
    }

    /* basic configuration */
    lowlevel_init();
    /* call the application's entry point */
    main();

    /* NOTREACHED */
    for(;;);
}

/*
 * basic configuraiton
 */
static void lowlevel_init()
{
    /* switch clock to sysclk */
    LPC_SC->CCLKSEL   = CCLKSEL_CCLKSEL_SYSCLK | CCLKSEL_CCLKDIV(1);
    /* turn-off USB clock */
    LPC_SC->USBCLKSEL = 0;

    /* turn-off PLL */
    LPC_SC->PLL0CON = 0;
    pllfeed();

    /* setup flash access time for 100 MHz to 120 MHz CPU clock (with power boost on) */
    LPC_SC->FLASHCFG |= 3 << 12;

    /* enable/disable power boost */
#if (CLK_CCLK > 100000000)
    LPC_SC->PBOOST = PBOOST_BOOST;
#else
    LPC_SC->PBOOST &= ~PBOOST_BOOST;
#endif

    /* select frequency range of main oscillator from 1 MHz to 20 Mhz */
    LPC_SC->SCS = 0;
    LPC_SC->SCS = SCS_OSCEN;
    /* wait for ready of main oscilator */
    while (!(LPC_SC->SCS & SCS_OSCSTAT));

    /* select main oscillator as the PLL0 clock source */
    LPC_SC->CLKSRCSEL = CLKSRCSEL_MAINOSC;
    /*
     * setup divider and multiplier
     *
     * 10    MHz  < pll_in_clk  < 25  MHz
     * 156   MHz  < Fcco        < 320 MHz
     * 9.75  MHz  < pll_out_clk < 160 MHz
     *
     *
     * M   1..32
     * P   1, 2, 4, 8
     *
     *
     * Fcco = pll_in_clk * M * 2 * P
     * pll_out_clk = Fcco / (2 * P)
     * 
     * M = pll_out_clk / pll_in_clk
     * 
     *
     */
    LPC_SC->PLL0CFG = (CLK_M - 1) | ((CLK_P - 1) << 5);
    pllfeed();
    /* enable PLL */
    LPC_SC->PLL0CON = PLLCON_PLLE;
    pllfeed();
    /* wait for enable */
    while(!(LPC_SC->PLL0STAT & PLLSTAT_PLLE));

    /* setup necessary clock dividers (CCLKSEL, PCLKSEL, EMCCLKSEL, USBCLKSEL) */
    LPC_SC->CCLKSEL   = CCLKSEL_CCLKSEL_PLL | CCLKSEL_CCLKDIV(CLK_CCLKDIV);
    LPC_SC->PCLKSEL   = PCLKSEL_PCLKDIV(CLK_PCLKDIV);
#if CLK_EMCDIV == 2
    LPC_SC->EMCCLKSEL = EMCCLKSEL_EMCDIV_DIV2;
#else
    LPC_SC->EMCCLKSEL = EMCCLKSEL_EMCDIV_CPU;
#endif

    /* wait for the PLL to lock */
    while(!(LPC_SC->PLL0STAT & PLLSTAT_PLOCK));

    /* connect PLL (CCLKSEL, USBCLKSEL, ... ) */
    LPC_SC->CCLKSEL |= CCLKSEL_CCLKSEL_PLL;
}

/*
 * PLL feed sequence
 */
static void pllfeed()
{
    LPC_SC->PLL0FEED = 0xAA;
    LPC_SC->PLL0FEED = 0x55;
}

