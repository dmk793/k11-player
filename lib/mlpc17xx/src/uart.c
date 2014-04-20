/*
 * generic uart functions
 */
#include <string.h>
#include "uart.h"
#include <LPC177x_8x_bits.h>
#include "debug.h"

struct uart_t uart[UART_COUNT];
static void uart_defaultisr(void *u);

/*
 * setup uart with specific parameters
 * and fill uart_t structure
 *
 * ARGS
 *
 * RETURN
 *      zero on success, error code otherwise
 *
 */
uint32 uart_setup(struct uart_t *pu, uint32 speed, struct uart_cb_t *cb)
{
    pu->isr = uart_defaultisr;
    if (cb)
    {
        pu->cb.rxchar = cb->rxchar;
        pu->cb.txend  = cb->txend;
    }

    if (pu == &uart[0])
    {
        LPC_SC->PCONP |= PCONP_PCUART0;

        pu->pm  = (LPC_UART1_TypeDef*)LPC_UART0;

        /* P0.2 (TXD0), P0.3 (RXD0) */
        LPC_IOCON->P0_2 &= ~LPC_IOCON_FUNC_MASK;
        LPC_IOCON->P0_2 |= LPC_IOCON_FUNC(1);
        LPC_IOCON->P0_3 &= ~LPC_IOCON_FUNC_MASK;
        LPC_IOCON->P0_3 |= LPC_IOCON_FUNC(1);
    } else if (pu == &uart[1]) {
        LPC_SC->PCONP |= PCONP_PCUART1;

        pu->pm  = (LPC_UART1_TypeDef*)LPC_UART1;

        /* P0.15 (TXD1), P0.16 (RXD1) */
        LPC_IOCON->P0_15 &= ~LPC_IOCON_FUNC_MASK;
        LPC_IOCON->P0_15 |= LPC_IOCON_FUNC(1);
        LPC_IOCON->P0_16 &= ~LPC_IOCON_FUNC_MASK;
        LPC_IOCON->P0_16 |= LPC_IOCON_FUNC(1);
    } else if (pu == &uart[2]) {
        /* TODO */
    } else if (pu == &uart[3]) {
        /* TODO */
    } else {
            /* NOTREACHED */
            return 1;
    }

    pu->pm->LCR = UART_LCR_DLAB;

    /*
     *                         PCLK 
     * BAUD =  ----------------------------------------
     *         (16 * (256 * DLM + DLL) * (1 + DIV/MUL))
     *
     *
     */

    /*
     * bits of speed argument:
     *
     * 0..7   bits  DLM
     * 8..15  bits  DLL
     * 16..23 bits  DIV
     * 24..31 bits  MUL
     */
    {
        uint8 dlm;
        uint8 dll;
        uint8 div;
        uint8 mul;

        dlm = (speed >> 0) & 0xff;
        dll = (speed >> 8) & 0xff;
        div = (speed >> 16) & 0xff;
        mul = (speed >> 24) & 0xff;

        pu->pm->DLM = dlm;
        pu->pm->DLL = dll;
        pu->pm->FDR = (div << 0) | (mul << 4);
        pu->pm->LCR = (3 << 0);
    }

    /* enable receive and line status interrupts */
    if (pu != DEBUG_UART) /* XXX */
        pu->pm->IER = UART_IER_RBR | UART_IER_RLS;
        
    /* UART0 - debug port */
    if (pu == &uart[0])
    {
        /* Reset RX, TX FIFO's, activate FIFO's, 0 characters for CTI interrupt */
        pu->pm->FCR = (1 << 2) | (1 << 1) | (1 << 0) | (0 << 6);
    } else {
        /* Reset RX, TX FIFO's, activate FIFO's, 14 characters for CTI interrupt */
        pu->pm->FCR = (1 << 2) | (1 << 1) | (1 << 0) | (3 << 6);
    }

    if (pu == &uart[0])
    {
        /* UART0 - debug port, set highest priority */
//        NVIC_SetPriority(UART0_IRQn, 0);

//        NVIC_EnableIRQ(UART0_IRQn);
//#ifdef UART_USE_IRQ_PRIORITY
//        NVIC_SetPriority(UART0_IRQn, UART0_IRQ_PRIORITY);
//#endif
    } else if (pu == &uart[1]) {
        NVIC_EnableIRQ(UART1_IRQn);
//#ifdef UART_USE_IRQ_PRIORITY
//        NVIC_SetPriority(UART1_IRQn, UART1_IRQ_PRIORITY);
//#endif
    } else if (pu == &uart[2]) {
        NVIC_EnableIRQ(UART2_IRQn);
//#ifdef UART_USE_IRQ_PRIORITY
//        NVIC_SetPriority(UART2_IRQn, UART2_IRQ_PRIORITY);
//#endif
    } else if (pu == &uart[3]) {
        NVIC_EnableIRQ(UART3_IRQn);
//#ifdef UART_USE_IRQ_PRIORITY
//        NVIC_SetPriority(UART3_IRQn, UART3_IRQ_PRIORITY);
//#endif
    } else {
            /* NOTREACHED */
            return 3;
    }

    return 0;
}

/*
 *
 */
void uart_disable(struct uart_t *pu)
{
    pu->pm->IER = 0;

    if (pu == &uart[0])
    {
        NVIC_DisableIRQ(UART0_IRQn);
    } else if (pu == &uart[1]) {
        NVIC_DisableIRQ(UART1_IRQn);
    } else if (pu == &uart[2]) {
        NVIC_DisableIRQ(UART2_IRQn);
    } else if (pu == &uart[3]) {
        NVIC_DisableIRQ(UART3_IRQn);
    }
}

/*
 * default uart IRQ handler
 */
static void uart_defaultisr(void *u)
{
    uint32 IIR, LSR;
    uint32 intid;
    struct uart_t *pu;

    pu = (struct uart_t *)u;

    IIR   = pu->pm->IIR;
    LSR   = pu->pm->LSR;
    intid = (IIR >> 1) & 0x07;

    if (intid == UART_INTID_RLS)
    {
        if (LSR & UART_LSR_RDR)
        {
            if (pu->rxbuf.c >= UARTBUF_SIZE)
            {
                pu->rxbuf.c = 0;
                dprint("sn", "overbuffer!");
            }
            pu->rxbuf.buf[pu->rxbuf.c++] = pu->pm->RBR;
        }
    } else if (intid == UART_INTID_RDA || intid == UART_INTID_CTI) {
        if (LSR & UART_LSR_RDR)
        {
            if (pu->rxbuf.c >= UARTBUF_SIZE) /* overbuffer ! */
            {
                pu->rxbuf.c = 0;
                dprint("sn", "overbuffer!");
            } else {
                pu->rxbuf.buf[pu->rxbuf.c++] = pu->pm->RBR;
                if (pu->cb.rxchar)
                    (*pu->cb.rxchar)(pu);
            }
        }
    } else if (intid == UART_INTID_THRE) {
        if (pu->txbuf.c > 0)
        {
            pu->pm->THR = pu->txbuf.buf[pu->txbuf.p++];
            pu->txbuf.c--;
        } else {
            if (pu->cb.txend)
                (*pu->cb.txend)(pu);
            pu->pm->IER &= ~UART_IER_THRE;
        }
    }
}

/*
 * for printf 
 */
int putChar(int c) 
{ 
    while (!(DEBUG_UART->pm->LSR & UART_LSR_THRE))
        ;
    DEBUG_UART->pm->THR = c;

    return c;
}

///*
// * check if transmission already in progress
// *
// * RETURN
// *     zero if uart is free, >0 ozerwise
// *
// * XXX
// *     Try to use bariers and readback interrupt enable bit to
// *     disable THRE interrupt.
// *     
// */
//uint8 uart_istx(struct uart_t *pu)
//{
//    uint8 ret;
////    uint32 IER;
//
//    __disable_irq();
//    if (pu->txbuf.c)
//        ret = 1;
//    else
//        ret = 0;
//    __enable_irq();
//
////    IER = pu->pm->IER;
////    if (IER & UART_IER_THRE)
////        return 1;
//
//    return ret;
//}
//
///*
// * initiate uart transmission
// * no check of transmission in progress
// */
//void uart_send(struct uart_t *pu)
//{
//    if (pu->txbuf.c > 0 && pu->txbuf.c <= UARTBUF_SIZE)
//    {
//        pu->txbuf.p = 0;
//        pu->pm->THR = pu->txbuf.buf[0];
//
//        pu->txbuf.c--;
//        pu->txbuf.p++;
//        pu->pm->IER |= UART_IER_THRE;
//    } else {
//        /* NOTREACHED */
//        debug_str("uart_send, wrong count\r\n");
//    }
//}
//
///*
// * send buffer to uart
// */
//void uart_sendbuf(struct uart_t *pu, uint8 *buf, uint32 c)
//{
//    if (c <= UARTBUF_SIZE)
//    {
//        memcpy(pu->txbuf.buf, buf, c);
//        uart_settxc(pu, c);
//        uart_send(pu);
//    } else
//        debug_str("uart_sendbuf, too long\r\n");
//}
//
/*
 *
 */
void UART0_Handler()
{
    struct uart_t *pu;

    pu = &uart[0];

    if (pu != NULL)
    {
        (*pu->isr)(pu);
    } else {
        /* NOTREACHED */
        debug_str("uart0, undef\r\n");
    }
}

/*
 *
 */
void UART1_Handler()
{
    struct uart_t *pu;

    pu = &uart[1];

    if (pu != NULL)
    {
        (*pu->isr)(pu);
    } else {
        /* NOTREACHED */
        debug_str("uart1, undef\r\n");
    }
}

/*
 *
 */
void UART2_Handler()
{
    struct uart_t *pu;

    pu = &uart[2];

    if (pu != NULL)
    {
        (*pu->isr)(pu);
    } else {
        /* NOTREACHED */
        debug_str("uart2, undef\r\n");
    }

}

/*
 *
 */
void UART3_Handler()
{
    struct uart_t *pu;

    pu = &uart[3];

    if (pu != NULL)
    {
        (*pu->isr)(pu);
    } else {
        /* NOTREACHED */
        debug_str("uart3, undef\r\n");
    }
}

///*
// * get count of bytes in receive buffer
// */
//uint32 uart_rxc(struct uart_t *pu)
//{
//    uint32 ret;
//
//    __disable_irq();
//    ret = pu->rxbuf.c;
//    __enable_irq();
//
//    return ret;
//}
//
