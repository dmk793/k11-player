#ifndef UART_H
#define UART_H

#include <LPC177x_8x.h>
#include <types.h>
#include <clk_cfg.h>

#define UART_BAUD_MAX_25_MHZ        ((0 << 0) | (1 << 8) | (0 << 16) | (1 << 24))
#define UART_BAUD_1562500_25_MHZ    UART_BAUD_MAX_25_MHZ
#define UART_BAUD_115200_25_MHZ     ((0 << 0) | (9 << 8) | (1 << 16) | (2 << 24))
#define UART_BAUD_48000_25_MHZ      ((0 << 0) | (26 << 8) | (1 << 16) | (4 << 24))
#define UART_BAUD_38400_25_MHZ      ((0 << 0) | (29 << 8) | (2 << 16) | (5 << 24))
#define UART_BAUD_9600_25_MHZ       ((0 << 0) | (93 << 8) | (6 << 16) | (8 << 24))
#define UART_BAUD_4800_25_MHZ       ((0 << 0) | (186 << 8) | (6 << 16) | (8 << 24))

#define UART_BAUD_115200_22_11834_MHZ ((0 << 0) | (12 << 8) | (0 << 16) | (1 << 24))
#define UART_BAUD_9600_22_11834_MHZ   ((0 << 0) | (144 << 8) | (0 << 16) | (1 << 24))

#define UART_BAUD_38400_20_MHZ      ((0 << 0) | (19 << 8) | (5 << 16) | (7 << 24))
#define UART_BAUD_38400_24_MHZ      ((0 << 0) | (23 << 8) | (7 << 16) | (10 << 24))

#define UART_BAUD_115200_20_MHZ     ((0 << 0) | ( 8 << 8) | (5 << 16) | (14 << 24))
#define UART_BAUD_115200_24_MHZ     ((0 << 0) | ( 7 << 8) | (6 << 16) | ( 7 << 24))
#define UART_BAUD_115200_36_MHZ     ((0 << 0) | (13 << 8) | (1 << 16) | ( 2 << 24))
#define UART_BAUD_115200_60_MHZ     ((0 << 0) | (19 << 8) | (5 << 16) | ( 7 << 24))

#if   CLK_PCLK == (36 * 1000000)                 
    #define UART_BAUD_115200    UART_BAUD_115200_36_MHZ
#elif CLK_PCLK   == (24 * 1000000)
    #define UART_BAUD_38400     UART_BAUD_38400_24_MHZ
    #define UART_BAUD_115200    UART_BAUD_115200_24_MHZ
#elif CLK_PCLK == (20 * 1000000)                 
    #define UART_BAUD_38400     UART_BAUD_38400_20_MHZ
    #define UART_BAUD_115200    UART_BAUD_115200_20_MHZ
#else
    #error Unknown PCLK frequency
#endif

#define UARTBUF_SIZE 128

typedef void (*uart_isr_f)(void *);

struct uart_buf_t {
    uint8 buf[UARTBUF_SIZE];
    uint32 p;
    uint32 c;
};

struct uart_t {
    LPC_UART1_TypeDef *pm;
    uart_isr_f isr;

    /* miscellaneous callbacks */
    struct uart_cb_t {
        void (*rxchar)(struct uart_t *);
        void (*txend)(struct uart_t *);
    } cb;

    struct uart_buf_t rxbuf;
    struct uart_buf_t txbuf;
};

#define UART_COUNT 4
extern struct uart_t uart[UART_COUNT];

#define DEBUG_UART (&uart[0])

int putChar(int c);
uint32 uart_setup(struct uart_t *pu, uint32 speed, struct uart_cb_t *cb);
//void uart_send(struct uart_t *pu);
//void uart_sendbuf(struct uart_t *pu, uint8 *buf, uint32 c);
//void uart_disable(struct uart_t *pu);
//uint32 uart_rxc(struct uart_t *pu);
//uint8 uart_istx(struct uart_t *pu);

void UART0_Handler();
void UART1_Handler();
void UART2_Handler();
void UART3_Handler();

//#define uart_reset_rx(pu) {__disable_irq(); (pu)->rxbuf.c = 0; __enable_irq();}
//#define uart_rxbuf(pu) ((pu)->rxbuf.buf)
//#define uart_txbuf(pu) ((pu)->txbuf.buf)
//#define uart_settxc(pu, txc) ((pu)->txbuf.c = (txc))

#endif

