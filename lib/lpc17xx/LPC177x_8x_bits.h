#ifndef LPC17XX_BITS_H
#define LPC17XX_BITS_H

#define PCONP_PCLCD    (1 << 0)
#define PCONP_PCTIM0   (1 << 1)
#define PCONP_PCTIM1   (1 << 2)
#define PCONP_PCUART0  (1 << 3)
#define PCONP_PCUART1  (1 << 4)
#define PCONP_PCSSP1   (1 << 10)
#define PCONP_PCEMC    (1 << 11)
#define PCONP_PCGPIO   (1 << 15)
#define PCONP_PCTIM2   (1 << 22)
#define PCONP_PCSDC    (1 << 28)
#define PCONP_PCGPDMA  (1 << 29)
#define PCONP_PCENET   (1 << 30)
#define PCONP_PCUSB    (1 << 31)

#define PLLCON_PLLE  (1 << 0)

#define PLLSTAT_PLLE  (1 << 8)
#define PLLSTAT_PLOCK (1 << 10)

#define LPC_IOCON_FUNC_MASK    (0x07 << 0)
#define LPC_IOCON_MODE_MASK    (0x03 << 3)
#define LPC_IOCON_FILTER_MASK  (1    << 8)

#define LPC_IOCON_FUNC(val)    ((val) << 0)
#define LPC_IOCON_FUNC_SET(pin, val) {LPC_IOCON->pin &= ~LPC_IOCON_FUNC_MASK; LPC_IOCON->pin |= val;}

#define LPC_IOCON_MODE_NO_PULLUP_NO_PULLDOWN(pin) {LPC_IOCON->pin &= ~LPC_IOCON_MODE_MASK; LPC_IOCON->pin |= 0x00;}
#define LPC_IOCON_MODE_PULLDOWN(pin)              {LPC_IOCON->pin &= ~LPC_IOCON_MODE_MASK; LPC_IOCON->pin |= 0x01;}
#define LPC_IOCON_MODE_PULLUP(pin)                {LPC_IOCON->pin &= ~LPC_IOCON_MODE_MASK; LPC_IOCON->pin |= 0x02;}
#define LPC_IOCON_MODE_REPEATER(pin)              {LPC_IOCON->pin &= ~LPC_IOCON_MODE_MASK; LPC_IOCON->pin |= 0x03;}

/* only for type A and W pins */
#define LPC_IOCON_MODE_FILTER_ON(pin)             {LPC_IOCON->pin &= ~LPC_IOCON_FILTER_MASK}
#define LPC_IOCON_MODE_FILTER_OFF(pin)            {LPC_IOCON->pin |=  LPC_IOCON_FILTER_MASK}


#define LPC_IOCON_P0_12_SSP1_MISO      2
#define LPC_IOCON_P0_13_SSP1_MOSI      2
#define LPC_IOCON_P0_14_USB_CONNECT2   3
#define LPC_IOCON_P0_29_EINT0          2
#define LPC_IOCON_P0_31_USB_DP2        1

#define LPC_IOCON_P1_2_SD_CLK          2
#define LPC_IOCON_P1_3_SD_CMD          2
#define LPC_IOCON_P1_6_SD_DAT0         2
#define LPC_IOCON_P1_7_SD_DAT1         2
#define LPC_IOCON_P1_11_SD_DAT2        2
#define LPC_IOCON_P1_12_SD_DAT3        2
#define LPC_IOCON_P1_19_SSP1_SCK       5
#define LPC_IOCON_P1_30_USB_PWRD2      1
#define LPC_IOCON_P1_30_USB_VBUS       2
#define LPC_IOCON_P1_31_SSP1_SCK       2


/****************************
 * UART
 ****************************/
#define UART_LCR_DLAB    (1 << 7)

#define UART_IER_RBR     (1 << 0)
#define UART_IER_THRE    (1 << 1)
#define UART_IER_RLS     (1 << 2)

#define UART_INTID_RLS   0x03
#define UART_INTID_RDA   0x02
#define UART_INTID_CTI   0x06
#define UART_INTID_THRE  0x01
#define UART_LSR_TEMT    (1 << 6)

#define UART_LSR_RDR     (1 << 0)
#define UART_LSR_THRE    (1 << 5)

#define TCR_ENABLE  (1 << 0)
#define TCR_RESET   (1 << 1)

/****************************
 * SSP
 ****************************/
#define SSP_CR0_DSS_4BIT       (3  << 0)       /* data size select */
#define SSP_CR0_DSS_5BIT       (4  << 0)       /* data size select */
#define SSP_CR0_DSS_6BIT       (5  << 0)       /* data size select */
#define SSP_CR0_DSS_7BIT       (6  << 0)       /* data size select */
#define SSP_CR0_DSS_8BIT       (7  << 0)       /* data size select */
#define SSP_CR0_DSS_9BIT       (8  << 0)       /* data size select */
#define SSP_CR0_DSS_10BIT      (9  << 0)       /* data size select */
#define SSP_CR0_DSS_11BIT      (10 << 0)       /* data size select */
#define SSP_CR0_DSS_12BIT      (11 << 0)       /* data size select */
#define SSP_CR0_DSS_13BIT      (13 << 0)       /* data size select */
#define SSP_CR0_DSS_14BIT      (14 << 0)       /* data size select */
#define SSP_CR0_DSS_15BIT      (15 << 0)       /* data size select */
#define SSP_CR0_DSS_16BIT      (16 << 0)       /* data size select */
#define SSP_CR0_FRF_SPI        (0  << 4)       /* SPI frame format */
#define SSP_CR0_FRF_TI         (1  << 4)       /* TI frame format */
#define SSP_CR0_FRF_MICROWIRE  (2  << 4)       /* Microwire frame format */
#define SSP_CR0_CPOL           (1  << 6)       /*  */
#define SSP_CR0_CPHA           (1  << 7)       /*  */
#define SSP_CR0_SCR(val)       (((val) & 0xff) << 8)

#define SSP_CR1_LBM       (1 << 0)
#define SSP_CR1_SSE       (1 << 1)
#define SSP_CR1_MS_MASTER (0 << 2)
#define SSP_CR1_MS_SLAVE  (1 << 2)
#define SSP_CR1_SOD       (1 << 3) /* slave output disable */

#define SSP_CPSR_CPSDVSR(val) ((val) & 0xff)

#define SSP_SR_TFE    (1 << 0) /* transmit FIFO empty */
#define SSP_SR_TNF    (1 << 1) /* transmit FIFO not full */
#define SSP_SR_RNE    (1 << 2) /* receive FIFO not empty */
#define SSP_SR_RFF    (1 << 3) /* receive FIFO full */
#define SSP_SR_BSY    (1 << 4) /* busy */
/* Interrupt Mask Set/Clear Register */
#define SSP_IMSC_RORIM  (1 << 0) /* receive overrun */
#define SSP_IMSC_RTIM   (1 << 1) /* receive timeout */
#define SSP_IMSC_RXIM   (1 << 2) /* RX FIFO half full */
#define SSP_IMSC_TXIM   (1 << 3) /* TX FIFO half empty */
/* Raw Interrupt Status Register */
#define SSP_RIS_RORRIS  (1 << 0) /* receive overrun */
#define SSP_RIS_RTRIS   (1 << 1) /* receive timeout */
#define SSP_RIS_RXRIS   (1 << 2) /* RX FIFO half full */
#define SSP_RIS_TXRIS   (1 << 3) /* TX FIFO half empty */
/* Interrupt Clear Register */
#define SSP_ICR_RORIC  (1 << 0) /* receive overrun */
#define SSP_ICR_RTIC   (1 << 1) /* receive timeout */
/* Masked Interrupt Status Register */
#define SSP_MIS_RORMIS  (1 << 0) /* receive overrun */
#define SSP_MIS_RTMIS   (1 << 1) /* receive timeout */
#define SSP_MIS_RXMIS   (1 << 2) /* RX FIFO half full */
#define SSP_MIS_TXMIS   (1 << 3) /* TX FIFO half empty */

#endif

//#define PCONP_PCPWM1   (1 << 6)
//#define PCONP_SSP1     (1 << 10)
//#define PCONP_PCMCPWM  (1 << 17)
//#define PCONP_SSP0     (1 << 21)
//#define PCONP_PCENET   (1 << 30)
//#define PCONP_PCUSB    (1 << 31)
//
///*
// * TIM0/1/2/3 bits
// */
//
///*
// * PWM, MCPWM bits
// */
//#define PWM1_TCR_COUNTERENABLE (1 << 0)
//#define PWM1_TCR_COUNTERRESET  (1 << 1)
//#define PWM1_TCR_PWMENABLE     (1 << 3)
//
//#define PWM1_MCR_MR0I   (1 << 0)
//#define PWM1_MCR_MR0R   (1 << 1)
//#define PWM1_MCR_MR0S   (1 << 2)
//#define PWM1_MCR_MR3I   (1 << 9)
//#define PWM1_MCR_MR3R   (1 << 10)
//#define PWM1_MCR_MR3S   (1 << 11)
//
//#define PWM1_PCR_SEL2   (1 << 2)
//#define PWM1_PCR_SEL3   (1 << 3)
//#define PWM1_PCR_SEL4   (1 << 4)
//#define PWM1_PCR_SEL5   (1 << 5)
//#define PWM1_PCR_SEL6   (1 << 6)
//
//#define PWM1_PCR_ENA1   (1 << 9)
//#define PWM1_PCR_ENA2   (1 << 10)
//#define PWM1_PCR_ENA3   (1 << 11)
//#define PWM1_PCR_ENA4   (1 << 12)
//#define PWM1_PCR_ENA5   (1 << 13)
//#define PWM1_PCR_ENA6   (1 << 14)
//
//#define PWM1_LER_LATCH0 (1 << 0)
//#define PWM1_LER_LATCH1 (1 << 1)
//#define PWM1_LER_LATCH2 (1 << 2)
//#define PWM1_LER_LATCH3 (1 << 3)
//#define PWM1_LER_LATCH4 (1 << 4)
//#define PWM1_LER_LATCH5 (1 << 5)
//#define PWM1_LER_LATCH6 (1 << 6)
//
//#define MCPWM_MCCON_RUN0    (1 << 0)
//#define MCPWM_MCCON_POLA0   (1 << 2)
//#define MCPWM_MCCON_RUN1    (1 << 8)
//#define MCPWM_MCCON_POLA1   (1 << 10)
//#define MCPWM_MCCON_RUN2    (1 << 16)
//#define MCPWM_MCCON_CENTER2 (1 << 17)
//#define MCPWM_MCCON_POLA2   (1 << 18)
//#define MCPWM_MCCON_DTE2    (1 << 19)
//#define MCPWM_MCCON_DISUP2  (1 << 20)
//


