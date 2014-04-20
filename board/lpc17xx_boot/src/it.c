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
 * interrupt vectors definitions
 */
extern unsigned int *_estack;        /* init value for the stack pointer. defined in linker script */

extern void Reset_Handler(void);

void Default_Handler(void);

void NMI_Handler(void);
void HF_Handler(void);
void MEM_Handler(void);
void BUS_Handler(void);
void UF_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);

void DPort_Handler(void);
void UART1_Handler(void);
void UART2_Handler(void);
void UART3_Handler(void);

/*
 * vector table
 */
unsigned int * myvectors[] __attribute__ ((section("vectors")))= {
    (unsigned int *)&_estack,             /* The initial stack pointer */
                                          /* Interrupt ID / Exception number, TODO Check interrupts sources comment, add missing interrupts */
    (unsigned int *)Reset_Handler,        /*      1,  Reset                       */
    (unsigned int *)NMI_Handler,          /*      2,  NMI                         */
    (unsigned int *)HF_Handler,           /*      3,  Hard Fault                  */
    (unsigned int *)MEM_Handler,          /*      4,  Memory Management           */
    (unsigned int *)BUS_Handler,          /*      5,  Bus Fault                   */
    (unsigned int *)UF_Handler,           /*      6,  Usage Fault                 */
    (unsigned int *)Default_Handler,      /*      7,  Reserved                    */
    (unsigned int *)Default_Handler,      /*      8,  Reserved                    */
    (unsigned int *)Default_Handler,      /*      9,  Reserved                    */
    (unsigned int *)Default_Handler,      /*      10, Reserved                    */
    (unsigned int *)Default_Handler,      /*      11, SVCall                      */
    (unsigned int *)DPort_Handler,        /*      12, Debug Monitor               */
    (unsigned int *)Default_Handler,      /*      13, Reserved                    */
    (unsigned int *)Default_Handler,      /*      14, PendSV Handler              */
    (unsigned int *)Default_Handler,      /*      15, SysTick ISR Handler         */
    (unsigned int *)Default_Handler,      /* 0  / 16, WatchDog Timer              */
    (unsigned int *)Default_Handler,      /* 1  / 17, Timer 0                     */
    (unsigned int *)Default_Handler,      /* 2  / 18, Timer 1                     */
    (unsigned int *)Default_Handler,      /* 3  / 19, Timer 2                     */
    (unsigned int *)Default_Handler,      /* 4  / 20, Timer 3                     */
    (unsigned int *)DPort_Handler,        /* 5  / 21, UART 0                      */
    (unsigned int *)UART1_Handler,        /* 6  / 22, UART 1                      */
    (unsigned int *)UART2_Handler,        /* 7  / 23, UART 2                      */
    (unsigned int *)UART3_Handler,        /* 8  / 24, UART 3                      */
    (unsigned int *)Default_Handler,      /* 9  / 25, PWM Interrupt               */
    (unsigned int *)Default_Handler,      /* 10 / 26, I2C0                        */
    (unsigned int *)Default_Handler,      /* 11 / 27, I2C1                        */
    (unsigned int *)Default_Handler,      /* 12 / 28, I2C2                        */
    (unsigned int *)Default_Handler,      /* 13 / 29, SPI0                        */
    (unsigned int *)Default_Handler,      /* 14 / 30, SSP0                        */
    (unsigned int *)Default_Handler,      /* 15 / 31, SSP1                        */
    (unsigned int *)Default_Handler,      /* 16 / 32, PLL lock                    */
    (unsigned int *)Default_Handler,      /* 17 / 33, Real Time Clock Interrupt                */
    (unsigned int *)Default_Handler,      /* 18 / 34, External interrupt 0                     */
    (unsigned int *)Default_Handler,      /* 19 / 35, External interrupt 1                     */
    (unsigned int *)Default_Handler,      /* 20 / 36, External interrupt 2                     */
    (unsigned int *)Default_Handler,      /* 21 / 37, External interrupt 3                     */
    (unsigned int *)Default_Handler,      /* 22 / 38, A/D Converter 0 end of conversion        */
    (unsigned int *)Default_Handler,      /* 23 / 39, Brown out detect                         */
    (unsigned int *)Default_Handler,      /* 24 / 40, USB DMA Interrupt                        */
    (unsigned int *)Default_Handler,      /* 25 / 41, CAN Interrupt                            */
    (unsigned int *)Default_Handler,      /* 26 / 42, IntStatus of DMA channel 0/1             */
    (unsigned int *)Default_Handler,      /* 27 / 43, SI (state change)                        */
    (unsigned int *)Default_Handler,      /* 28 / 44, Ethernet Interrupt                       */
    (unsigned int *)Default_Handler,      /* 29 / 45, Repetitive interrupt timer               */
    (unsigned int *)Default_Handler,      /* 30 / 46, Motor Control PWM                        */
    (unsigned int *)Default_Handler,      /* 31 / 47, Quadrature Encoder                       */
    (unsigned int *)Default_Handler,      /* 32 / 48, PLL1 lock (PLOCK)                        */
    (unsigned int *)Default_Handler,      /* 33 / 49, USB Activity                             */
    (unsigned int *)Default_Handler,      /* 34 / 50, CAN Activity                             */
};

/*
 *
 */
void Default_Handler(void)
{
    while (1);
}

/*
 *
 */
void NMI_Handler(void)
{
    while (1);
}

/*
 *
 */
void HF_Handler(void)
{
    while (1);
}

/*
 *
 */
void MEM_Handler(void)
{
    while (1);
}

/*
 *
 */
void BUS_Handler(void)
{
    while (1);
}

/*
 *
 */
void UF_Handler(void)
{
    while (1);
}

