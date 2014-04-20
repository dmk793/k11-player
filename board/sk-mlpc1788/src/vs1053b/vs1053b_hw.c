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

#include <debug.h>
#include <string.h>
#include <LPC177x_8x.h>
#include <LPC177x_8x_bits.h>
#include <clk_cfg.h>
#include <os.h>
#include <gpio.h>
#include <stimer.h>
#include "../irqp.h"
#include "vs1053b_hw.h"

#define DEBUG_VS1053B_HW

#ifdef DEBUG_VS1053B_HW
    #define DPRINT(fmt, ...)  dprint(fmt, __VA_ARGS__)
#else
    #define DPRINT(fmt, ...)
#endif



#define VSGPIO_COUNT   4
static const struct gpio_t vsgpio[VSGPIO_COUNT] = {
    {LPC_GPIO2, (1 << 27)}, 
    {LPC_GPIO2, (1 << 31)}, 
//    {LPC_GPIO0, (1 << 28)}, 
    {LPC_GPIO0, (1 << 29)}, 
    {LPC_GPIO2, (1 << 30)}, 
};

#define VSGPIO_XCS    (&vsgpio[0])
#define VSGPIO_XDCS   (&vsgpio[1])
#define VSGPIO_DREQ   (&vsgpio[2])
#define VSGPIO_XRESET (&vsgpio[3])

#define VS1053B_RESET_ON()  gpio_drive(VSGPIO_XRESET, 0)
#define VS1053B_RESET_OFF() gpio_drive(VSGPIO_XRESET, 1)
#define VS1053B_GET_DREQ()  gpio_read(VSGPIO_DREQ)
#define VS1053B_XCS_ON()    gpio_drive(VSGPIO_XCS, 0)
#define VS1053B_XCS_OFF()   gpio_drive(VSGPIO_XCS, 1)
#define VS1053B_XDCS_ON()   gpio_drive(VSGPIO_XDCS, 0)
#define VS1053B_XDCS_OFF()  gpio_drive(VSGPIO_XDCS, 1)

#define INIT_TO    1000

#define SSP1_EVENT_DONE      (1 << 0)
#define EINT_EVENT_DREQ_HIGH (1 << 1)
static BASE_TYPE mevent;

/*
 *
 */
int vs1053b_hw_init()
{
    uint32 to;

    /*
     * Available SSP:
     *     SSP0
     *         SCK   P0.15, P1.20, P2.22
     *         MISO  P0.17, P1.23, P2.26
     *         MOSI  P0.18, P1.24, P2.27
     *         SSEL  P0.16, P1.21, P1.28, P2.23
     *
     *     SSP1
     *         SCK   P0.7, P1.19, P1.31, P4.20
     *         MISO  P0.8, P0.12, P1.18, P4.22
     *         MOSI  P0.9, P0.13, P1.22, P4.23
     *         SSEL  P0.6, P0.14, P1.26, P4.21
     *
     *     SSP2
     *         SCK   P1.0
     *         MISO  P1.4, P5.1
     *         MOSI  P1.1, P5.0
     *         SSEL  P1.8
     *
     * Used pins:
     * 
     * SSP1
     *     SCK     P1.31    MP40       <---> SCLK
     *     MISO    P0.12    MP41       <---- SO
     *     MOSI    P0.13    MP45       ----> SI
     *
     *             P2.27    MP47       ----> xCS
     *             P2.31    MP39       ----> xDCS
     *             P0.28    MP48       <---- DREQ   XXX first variant
     *             P0.29    UA_P       <---- DREQ   XXX second variant (P0.29, EINT0), XXX cross with UA_P
     *             P2.30    MP31       ----> xRESET
     */
    LPC_IOCON_FUNC_SET(P1_31, LPC_IOCON_P1_31_SSP1_SCK);
    LPC_IOCON_FUNC_SET(P0_12, LPC_IOCON_P0_12_SSP1_MISO);
    LPC_IOCON_FUNC_SET(P0_13, LPC_IOCON_P0_13_SSP1_MOSI);

    {
        LPC_IOCON_FUNC_SET(P0_29, LPC_IOCON_P0_29_EINT0);

        NVIC_SetPriority(EINT0_IRQn, EINT0_IRQP);
        NVIC_ClearPendingIRQ(EINT0_IRQn);
        NVIC_DisableIRQ(EINT0_IRQn);

        LPC_SC->EXTINT    =  (1 << 0); /* clear IRQ       */
        LPC_SC->EXTMODE  &= ~(1 << 0); /* level sensitive */
        LPC_SC->EXTPOLAR |=  (1 << 0); /* high level sensitive */
        LPC_SC->EXTINT    =  (1 << 0); /* clear IRQ       */
    }

    gpio_setdir(VSGPIO_XCS,    GPIO_DIR_OUTPUT);
    gpio_setdir(VSGPIO_XDCS,   GPIO_DIR_OUTPUT);
    gpio_setdir(VSGPIO_DREQ,   GPIO_DIR_INPUT);
    gpio_setdir(VSGPIO_XRESET, GPIO_DIR_OUTPUT);

    VS1053B_XCS_OFF();
    VS1053B_XDCS_OFF();

    DEBUG_IMSG("hw reset");
    vs1053b_hw_reset_on();

    LPC_SC->PCONP |= PCONP_PCSSP1;

/*
 * Freq = PCLK / (CPSDVSR * (SCR + 1))
 *
 * speed bits:
 *     0..7    CPSDVSR  (even values between 2 and 254)
 *     8..15   SCR
 *
 *
 * SCI reads                 CLKI/7
 * SDI and SDI writes        CLKI/4
 */

#if   CLK_PCLK == (36 * 1000 * 1000)
    #define SSP_SPEED_1MHZ_36MHZ     (6   | (5 << 8))
    #define SSP_SPEED_18MHZ_36MHZ    (2   | (0 << 8))


    #define SSP_LO_SPEED SSP_SPEED_1MHZ_36MHZ
    #define SSP_HI_SPEED SSP_SPEED_18MHZ_36MHZ
#elif CLK_PCLK == (24 * 1000 * 1000)
    #define SSP_SPEED_12MHZ_24MHZ    (2   | (0 << 8))
    #define SSP_SPEED_6MHZ_24MHZ     (2   | (1 << 8))
    #define SSP_SPEED_3MHZ_24MHZ     (4   | (1 << 8))
    #define SSP_SPEED_2MHZ_24MHZ     (6   | (1 << 8))
    #define SSP_SPEED_1_5MHZ_24MHZ   (4   | (3 << 8))
    #define SSP_SPEED_1MHZ_24MHZ     (12  | (1 << 8))
    #define SSP_SPEED_500KHZ_24MHZ   (12  | (3 << 8))

    #define SSP_LO_SPEED SSP_SPEED_1MHZ_24MHZ
    #define SSP_HI_SPEED SSP_SPEED_6MHZ_24MHZ

#elif CLK_PCLK == (20 * 1000 * 1000)
    #define SSP_SPEED_10MHZ_20MHZ    (2  | (0 << 8))
    #define SSP_SPEED_5MHZ_20MHZ     (2  | (1 << 8))
    #define SSP_SPEED_1_25MHZ_20MHZ  (8  | (1 << 8))
    #define SSP_SPEED_625KHZ_20MHZ   (8  | (3 << 8))

    #define SSP_LO_SPEED SSP_SPEED_625KHZ_20MHZ
    #define SSP_HI_SPEED SSP_SPEED_10MHZ_20MHZ
#else
    #error Unknown PCLK frequency
#endif

    #define SSP_INITIAL_SPEED SSP_LO_SPEED

    LPC_SSP1->CR1  = 0;
    LPC_SSP1->CR0  = SSP_CR0_DSS_8BIT | SSP_CR0_FRF_SPI | SSP_CR0_SCR(SSP_INITIAL_SPEED >> 8); 
    LPC_SSP1->CPSR = SSP_CPSR_CPSDVSR(SSP_INITIAL_SPEED & 0xff);
    LPC_SSP1->CR1 = SSP_CR1_SSE;

    NVIC_SetPriority(SSP1_IRQn, SSP1_IRQP);
    NVIC_ClearPendingIRQ(SSP1_IRQn);

    vs1053b_hw_reset_off();

    DEBUG_IMSG("wait");

    /* wait a while */
    os_wait_ms(100);

    stimer_settime(&to);
    while (!VS1053B_GET_DREQ() && stimer_deltatime(to) < INIT_TO)
        os_wait(1);

    if (stimer_deltatime(to) >= INIT_TO)
    {
        DEBUG_EMSG("DREQ wait timeout");
        return 0;
    }
    DEBUG_IMSG("DREQ high");

    return 1;
}

/*
 *
 */

struct ssp_state_t {
#define SSP_BUFSIZE    64
    uint8 txbuf[SSP_BUFSIZE];
    uint8 rxbuf[SSP_BUFSIZE];
    int txlen;
    int rxlen;
    uint8 *txp;
    uint8 *rxp;
};

static struct ssp_state_t ssp_state;

void SSP1_Handler(void)
{
    if (LPC_SSP1->MIS & SSP_MIS_RTMIS)
        LPC_SSP1->ICR  = SSP_ICR_RTIC;
    if (LPC_SSP1->MIS & SSP_MIS_RORMIS)
        LPC_SSP1->ICR  = SSP_ICR_RORIC;

    while ((ssp_state.txlen && (LPC_SSP1->SR & SSP_SR_TNF)) || (LPC_SSP1->SR & SSP_SR_RNE))
    {
        if (ssp_state.txlen && (LPC_SSP1->SR & SSP_SR_TNF))
        {
            LPC_SSP1->DR = *ssp_state.txp++;
            ssp_state.txlen--;
        }

        while (LPC_SSP1->SR & SSP_SR_RNE)
        {
            *ssp_state.rxp++ = LPC_SSP1->DR;
            ssp_state.rxlen--;

            if (ssp_state.rxlen == 0)
            {
                LPC_SSP1->IMSC = 0;
                NVIC_DisableIRQ(SSP1_IRQn);
                os_event_raise(&mevent, SSP1_EVENT_DONE);
                return;
            }
        }
    }
}

/*
 *
 */
static void ssp_wr(uint8 *tx, uint8 *rx, uint32 len)
{
    int n;

    if (!len)
        return;

    LPC_SSP1->IMSC = 0;
    NVIC_DisableIRQ(SSP1_IRQn);

    while (len)
    {
        n = len;
        if (n > SSP_BUFSIZE)
            n = SSP_BUFSIZE;

        if (tx != NULL)
        {
            LPC_SSP1->DR = *tx;
            memcpy(ssp_state.txbuf, tx + 1, n - 1);
        } else {
            LPC_SSP1->DR = 0x5a;
        }

        ssp_state.txlen = n - 1;
        ssp_state.rxlen = n;
        ssp_state.txp   = ssp_state.txbuf;
        ssp_state.rxp   = ssp_state.rxbuf;

        LPC_SSP1->IMSC = SSP_IMSC_RXIM | SSP_IMSC_RTIM | SSP_IMSC_RORIM;
        NVIC_ClearPendingIRQ(SSP1_IRQn);
        NVIC_EnableIRQ(SSP1_IRQn);

        os_event_wait(&mevent, SSP1_EVENT_DONE, OS_FLAG_CLEAR, OS_WAIT_FOREVER);
//        os_event_wait(&event, SSP1_EVENT_DONE, OS_FLAG_CLEAR, OS_MS2TICK(50));

        len -= n;
        if (tx)
            tx += n;
        if (rx)
        {
            memcpy(rx, ssp_state.rxbuf, n);
            rx += n;
        }
    }
}

//static void ssp_wr(uint8 *tx, uint8 *rx, uint32 len)
//{
//    while (len--)
//    {
//        if (tx != NULL)
//            LPC_SSP1->DR = *tx++;
//        else
//            LPC_SSP1->DR = 0x5a;
//
//        /* XXX wait for tx/rx */
//        while ((LPC_SSP1->SR & (SSP_SR_RNE | SSP_SR_BSY)) != SSP_SR_RNE)
//            ;
//
//        if (rx != NULL)
//            *rx++ = LPC_SSP1->DR;
//        else
//            LPC_SSP1->DR;
//    }
//}

/*
 *
 */
void inline vs1053b_hw_set_hi_fclk()
{
    LPC_SSP1->CR1  = 0;
    LPC_SSP1->CR0  = SSP_CR0_DSS_8BIT | SSP_CR0_FRF_SPI | SSP_CR0_SCR(SSP_HI_SPEED >> 8); 
    LPC_SSP1->CPSR = SSP_CPSR_CPSDVSR(SSP_HI_SPEED & 0xff);
    LPC_SSP1->CR1 = SSP_CR1_SSE;
}

/*
 *
 */
void inline vs1053b_hw_set_low_fclk()
{
    LPC_SSP1->CR1  = 0;
    LPC_SSP1->CR0  = SSP_CR0_DSS_8BIT | SSP_CR0_FRF_SPI | SSP_CR0_SCR(SSP_LO_SPEED >> 8); 
    LPC_SSP1->CPSR = SSP_CPSR_CPSDVSR(SSP_LO_SPEED & 0xff);
    LPC_SSP1->CR1 = SSP_CR1_SSE;
}



/*
 *
 */
void inline vs1053b_hw_reset_on()
{
    VS1053B_RESET_ON();
}
void inline vs1053b_hw_reset_off()
{
    VS1053B_RESET_OFF();
}

/*
 *
 */
uint16 vs1053b_hw_readsci(uint8 addr)
{
    uint8 buf[4];

    /* wait when DREQ will go high */
    while (!VS1053B_GET_DREQ())
        os_wait(1);

//    DEBUG_IMSG("DREQ high");

    VS1053B_XCS_ON();
    {
        buf[0] = 0x03; /* read */
        buf[1] = addr;
        buf[2] = 0xff;
        buf[3] = 0xff;
        ssp_wr(buf, buf, 4);
    }
    VS1053B_XCS_OFF();

    return (buf[3] | (buf[2] << 8));
}

/*
 *
 */
void vs1053b_hw_writesci(uint8 addr, uint16 value)
{
    uint8 buf[4];

    /* wait when DREQ will go high */
    while (!VS1053B_GET_DREQ())
        os_wait(1);

//    DEBUG_IMSG("DREQ high");

    VS1053B_XCS_ON();
    {
        buf[0] = 0x02; /* write */
        buf[1] = addr;
        buf[2] = value >> 8;
        buf[3] = value & 0xff;
        ssp_wr(buf, NULL, 4);
    }
    VS1053B_XCS_OFF();
}

/*
 * NOTE no check of DREQ
 */
void vs1053b_hw_writesdi(uint8 *data, int len)
{
    VS1053B_XDCS_ON();
    ssp_wr(data, NULL, len);
    VS1053B_XDCS_OFF();
}

/*
 *
 */
int vs1053b_hw_check_dreq()
{
    return VS1053B_GET_DREQ();
}

/*
 *
 */
void vs1053b_hw_wait_dreq()
{
    while (!vs1053b_hw_check_dreq())
    {
        LPC_SC->EXTINT = (1 << 0); /* clear IRQ       */
        NVIC_ClearPendingIRQ(EINT0_IRQn);
        NVIC_EnableIRQ(EINT0_IRQn);
        os_event_wait(&mevent, EINT_EVENT_DREQ_HIGH, OS_FLAG_CLEAR, OS_WAIT_FOREVER);
    }
}

/*
 *
 */
void EINT0_Handler(void)
{
    if (vs1053b_hw_check_dreq())
    {
        os_event_raise(&mevent, EINT_EVENT_DREQ_HIGH);
        LPC_SC->EXTINT = (1 << 0); /* clear IRQ       */

        NVIC_DisableIRQ(EINT0_IRQn);
        NVIC_ClearPendingIRQ(EINT0_IRQn);
        return;
    }

    dprint("sn", "unhopped irq");
}

