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
 * TODO
 *     * turn on/off initializaiton of debug port dependant on GPIO state
 *       (jumper switch) (or exit from handler without process if GPIO in specific state)
 *     * show with LED DPort initialization                         
 *     * if C_DEBUGEN enabled in DHCSR then DebugMonitor exception
 *       does not work - show this also
 */
#include <LPC177x_8x.h>
#include <string.h>
#include <stimer.h>
#include "dport.h"
#include "dport_hw.h"
#include "dport_proto.h"

//#define STATIC static
#define STATIC

static void dport_process();
static void dport_handler();
static uint8 dport_cs(uint8 *cs, uint8 *data, int len);
static uint16 dport_num2dup(uint8 num);
static uint8 dport_dup2num(uint16 dup);
static void dport_send_data(uint8 cmd, uint8 *data, int len);
static int dport_process_cmd(uint8 cmd, uint8 *data, int len);


extern uint32 *_flash_start;
extern uint32 *_flash_size;
extern uint32 *_uvect_start;
extern uint32 *_uvect_size;

struct dport_context_t {
    /* context stored by software */
    uint32 r4;     /* 0x00 */
    uint32 r5;     /* 0x04 */
    uint32 r6;     /* 0x08 */
    uint32 r7;     /* 0x0c */
    uint32 r8;     /* 0x10 */
    uint32 r9;     /* 0x14 */
    uint32 r10;    /* 0x18 */
    uint32 r11;    /* 0x1c */
    /* context stored by ARMv7-M core */
    uint32 r0;     /* 0x20 0x00 */
    uint32 r1;     /* 0x24 0x04 */
    uint32 r2;     /* 0x28 0x08 */
    uint32 r3;     /* 0x2c 0x0c */
    uint32 r12;    /* 0x30 0x10 */
    uint32 lr;     /* 0x34 0x14 */
    uint32 pc;     /* 0x38 0x18 */
    uint32 xpsr;   /* 0x3c 0x1c */

    /* not realy used */
    uint32 sp;     /* 0x40 */
};

#define DPORT_BUF_SIZE    2048

struct dport_buffer_t {
    uint8 data[DPORT_BUF_SIZE];
    int p;
    int c;
};

struct dport_state_t {
    struct dport_buffer_t ibuf;

    uint32 coffset;
};

STATIC struct dport_context_t context;
STATIC struct dport_state_t   dstate;

/*
 *
 */
void dport_init()
{
    /* set handlers priorities to lowest */
    {
        int i;

        for (i = 0; i < sizeof(NVIC->IP); i++)
            NVIC->IP[i] = 0xff;
    }

    /* enable the DebugMonitor exception */
    CoreDebug->DEMCR = CoreDebug_DEMCR_MON_EN_Msk | CoreDebug_DEMCR_TRCENA_Msk;

    /*
     * if C_DEBUGEN enabled in DHCSR then DebugMonitor exception
     * does not work.
     */
    if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)
    {

    }

    dport_hw_init();

    dstate.ibuf.p = 0;
    dstate.ibuf.c = 0;

//    dport_send_data(DPORT_CMD_REQ_RESET, NULL, 0);
}

/*
 * Called on "Debug Monitor" exception and "UART0" interrupt
 *
 * NOTE
 *     main program dport vector point to redireciton function which
 *     redirect to corresponding bootloader vector
 *
 * NOTE interrupt disable not necessary if this handler has highest priority
 */
void  __attribute__((naked)) DPort_Handler()
{
    /*
     * store context
     *
     *    EXC_RETURN
     *    0xFFFFFFF1    Return to Handler mode.
     *                  Exception return gets state from the main stack.
     *                  Execution uses MSP after return.
     *    0xFFFFFFF9    Return to Thread mode.
     *                  Exception Return get state from the main stack.
     *                  Execution uses MSP after return.
     *    0xFFFFFFFD    Exception return gets state from the process stack.
     *                  Execution uses PSP after return.
     */
    asm volatile (
        /* store software part of context */
        "ldr   r1, dpcontext__ \n"
        "stmia r1!,{r4-r11}    \n"
        /* check whether MSP or PSP used by previous thread/handler */
        "mov   r0, #0xfffffffd \n"
        "cmp   lr, r0          \n"
        "ite   eq              \n"
        "mrseq r0, psp         \n"
        "mrsne r0, msp         \n"
        /* XXX store SP in context \n*/
        "str   sp, [r1, #0x20] \n"
        /* copy hardware stack to context (r0-r3, r12, lr, pc, xpsr) */
        "mov   r3, #8          \n"
        "copy_hw_stack:        \n"
        "\t ldr  r2, [r0], #4  \n"
        "\t str  r2, [r1], #4  \n"
        "\t subs r3, #1        \n"
        "\t bne  copy_hw_stack \n"
    );

    /* call handler */
    asm volatile (
        "push {lr}  \n" /* NOTE save LR before branch to handler */
        "mov r0, %0 \n"
        "blx r0     \n"
        "pop  {lr}  \n" /* NOTE restore LR */
         : : "r"(dport_handler) : "r0", "lr"
    );

    /* restore context */
    asm volatile (
        /* check whether MSP or PSP used by previous thread/handler */
        "mov   r0, #0xfffffffd  \n"
        "cmp   lr, r0           \n"
        "ite   eq               \n"
        "mrseq r0, psp          \n"
        "mrsne r0, msp          \n"
        /* restore hardware part of context (r0-r3, r12, lr, pc, xpsr) */
        "ldr   r1, dpcontext__  \n"
        "add   r1, #0x20        \n"
        "mov   r3, #8           \n"
        "copyback_hw_stack:     \n"
        "\t ldr  r2, [r1], #4   \n"
        "\t str  r2, [r0], #4   \n"
        "\t subs r3, #1         \n"
        "\t bne  copyback_hw_stack \n"
        /* restore software part of context */
        "ldr   r1, dpcontext__  \n"
        "ldmia r1!,{r4-r11}     \n"
        /* return from handler */
        "bx lr                  \n"
      	".align 2                   \n"
	"dpcontext__: .word context \n"
    );
}

/*
 *
 */
static void dport_handler()
{
    char irqn;

    /* if handler is UART0 handler wait for valid command */
    irqn = (SCB->ICSR & 0xff) - 16; 
    if (irqn == UART0_IRQn)
    {
//        dport_send_data('U', (uint8*)&context.pc, 4);
    } else {
//        CoreDebug->DEMCR |= CoreDebug_DEMCR_MON_STEP_Msk;

        /* XXX move to instruction next to BKPT */
        context.pc += 2;
    }

    dport_process();

    if (irqn != UART0_IRQn)
    {
        /* Debug Monitor Exception */
        SCB->DFSR = SCB_DFSR_BKPT_Msk; /* XXX clear BKPT event? */
    }
    NVIC_ClearPendingIRQ(UART0_IRQn);
}

/*
 *
 */
static void dport_process()
{
    char ch;
    int *cnt;
    int dlen;
    uint8 *buf;

    while (1)
    {
        if (!dport_hw_getchar(&ch))
            continue;

        /* TODO exit from handler on specific GPIO state */

        cnt = &dstate.ibuf.c;
        buf = dstate.ibuf.data;

        if (ch == DPORT_CMD_MARK)
            *cnt = 0;

        if (*cnt == 0 && ch != DPORT_CMD_MARK)
            continue;

        if (*cnt > DPORT_BUF_SIZE)
        {
            *cnt = 0;
            continue;
        }

        buf[*cnt] = ch;
        *cnt = *cnt + 1;
        if (ch == '\n')
        {
            /*
             * Command should have even number of symbols.
             * And at least 6 symbols - one symbol for MARK,
             * one symbol for command, two symbols for checksum; CR symbol,
             * LF symbol.
             */
            if ((*cnt % 2) != 0 || *cnt < DPORT_HEADTAIL_SIZE)
            {
                *cnt = 0;
                continue;
            }

            /* transform command to binary data */
            {
                uint8 *in;
                uint8 *out;

                in  = &buf[2];
                out = &buf[2];
                /* length of hexadecimal data transformed to binary */
                dlen = (*cnt - DPORT_HEADTAIL_SIZE) / 2;
                while (*in != '\r')
                {
                    *out = dport_dup2num(*((uint16*)in));
                    in   += 2;
                    out  += 1;
                }
            }

            /* check CS */
            {
                uint8 cs;

                cs = dport_cs(&buf[1], NULL, 0);
                cs = dport_cs(&cs, &buf[2], dlen);

                if (cs != buf[2 + dlen])
                {
                    dport_send_data(DPORT_CMD_RESP_CSERROR, NULL, 0);
                }
                else
                    if (dport_process_cmd(buf[1], &buf[2], dlen))
                        return;
            }

            *cnt = 0;
            continue;
        }
    }
}

/*
 * ARGS
 *     cmd    input command
 *     data   input data
 *     len    length of input data
 *     
 *
 * RETURN
 *     1 if debug handler should return control to main program
 */
static int dport_process_cmd(uint8 cmd, uint8 *data, int len)
{
    union dport_req_t  *req;
    union dport_resp_t *resp;

    req  = (union dport_req_t*)data;
    resp = (union dport_resp_t*)data; /* XXX same buffer */

    switch (cmd)
    {
        case DPORT_CMD_REQ_BUFSIZE:
            resp->bufsize.length = DPORT_BUF_SIZE;
            dport_send_data(cmd, (uint8*)resp, 2);
            return 1; /* NOTE continue */
        case DPORT_CMD_REQ_BREAK:
            dport_send_data(cmd, (uint8*)&context.pc, 4);
            return 0;
        case DPORT_CMD_REQ_CONTINUE:
            dport_send_data(cmd, NULL, 0);
            return 1;
        case DPORT_CMD_REQ_READREG:
            if (len < 1)
                return 0;
            switch (data[0])
            {
                /* XXX GDB numbering scheme */
                case  0: memcpy(&data[1], &context.r0,  4); break;
                case  1: memcpy(&data[1], &context.r1,  4); break;
                case  2: memcpy(&data[1], &context.r2,  4); break;
                case  3: memcpy(&data[1], &context.r3,  4); break;
                case  4: memcpy(&data[1], &context.r4,  4); break;
                case  5: memcpy(&data[1], &context.r5,  4); break;
                case  6: memcpy(&data[1], &context.r6,  4); break;
                case  7: memcpy(&data[1], &context.r7,  4); break;
                case  8: memcpy(&data[1], &context.r8,  4); break;
                case  9: memcpy(&data[1], &context.r9,  4); break;
                case 10: memcpy(&data[1], &context.r10, 4); break;
                case 11: memcpy(&data[1], &context.r11, 4); break;
                case 12: memcpy(&data[1], &context.r12, 4); break;
                case 13: memcpy(&data[1], &context.sp,  4); break;
                case 14: memcpy(&data[1], &context.lr,  4); break;
                case 15: memcpy(&data[1], &context.pc,  4); break;
                case 25: memcpy(&data[1], &context.xpsr,4); break;
                default: return 0;
            }
            dport_send_data(cmd, data, 5);

            return 0;
        case DPORT_CMD_REQ_WRITEREG:
            if (len < 1)
                return 0;
            switch (req->write_reg.regn)
            {
                /* XXX GDB numbering scheme */
                case  0: memcpy(&context.r0,   &data[1], 4); break;
                case  1: memcpy(&context.r1,   &data[1], 4); break;
                case  2: memcpy(&context.r2,   &data[1], 4); break;
                case  3: memcpy(&context.r3,   &data[1], 4); break;
                case  4: memcpy(&context.r4,   &data[1], 4); break;
                case  5: memcpy(&context.r5,   &data[1], 4); break;
                case  6: memcpy(&context.r6,   &data[1], 4); break;
                case  7: memcpy(&context.r7,   &data[1], 4); break;
                case  8: memcpy(&context.r8,   &data[1], 4); break;
                case  9: memcpy(&context.r9,   &data[1], 4); break;
                case 10: memcpy(&context.r10,  &data[1], 4); break;
                case 11: memcpy(&context.r11,  &data[1], 4); break;
                case 12: memcpy(&context.r12,  &data[1], 4); break;
                case 13: memcpy(&context.sp,   &data[1], 4); break;
                case 14: memcpy(&context.lr,   &data[1], 4); break;
                case 15: memcpy(&context.pc,   &data[1], 4); break;
                case 25: memcpy(&context.xpsr, &data[1], 4); break;
                default: return 0;
            }

            resp->write_reg.regn = req->write_reg.regn;
            dport_send_data(cmd, (uint8*)resp, sizeof(struct dport_resp_write_reg_t));
            return 0;
        case DPORT_CMD_REQ_READMEM:
            if (req->read_mem.length > READMEM_PAYLOAD_MAX_SIZE)
                return 0;
            resp->read_mem.offset = req->read_mem.offset;
            resp->read_mem.length = req->read_mem.length;
            switch (req->read_mem.length)
            {
                case 4:
                    resp->read_mem.reg32  = *((volatile uint32*)req->read_mem.offset);
                    break;
                case 2:
                    resp->read_mem.reg16 = *((volatile uint16*)req->read_mem.offset);
                    break;
                case 1:
                    resp->read_mem.reg8 = *((volatile uint8*)req->read_mem.offset);
                    break;
                default:
                    memcpy(resp->read_mem.data, (void*)req->read_mem.offset, req->read_mem.length);
            }

            dport_send_data(cmd, data, 8 + req->read_mem.length);
            return 0;
        case DPORT_CMD_REQ_WRITEMEM:
            if (req->write_mem.length > WRITEMEM_PAYLOAD_MAX_SIZE)
                return 0;

            resp->write_mem.offset = req->write_mem.offset;
            resp->write_mem.length = req->write_mem.length;

            switch (req->write_mem.length)
            {
                case 4:
                    *((volatile uint32*)req->write_mem.offset) = req->write_mem.reg32;
                    break;
                case 2:
                    *((volatile uint16*)req->write_mem.offset) = req->write_mem.reg16;
                    break;
                case 1:
                    *((volatile uint8*)req->write_mem.offset) = req->write_mem.reg8;
                    break;
                default:
                    memcpy((void*)(uint32)req->write_mem.offset, req->write_mem.data, req->write_mem.length);
            }

            asm volatile (
                "dsb \n"
                "isb \n"
            );

            dport_send_data(cmd, (uint8*)resp, sizeof(struct dport_write_mem_resp_t));
            return 0;
        case DPORT_CMD_REQ_FLASH:
            /* XXX simple, FLASHBUF_SIZE bytes blocks write with autoerase */
            {
                uint32 offset;
                uint32 result;
                uint32 length;
                uint32 fstart;
                uint32 fsize;

                offset = req->flash.offset;
                length = req->flash.length;

                fstart = (uint32)&_flash_start;
                fsize  = (uint32)&_flash_size;

                /* XXX flash range check */
                if ((fstart > 0 && offset < fstart) || 
                        ((offset + length) > (fstart + fsize)))
                {
                    result = 0xa0a0a0a0;
                    dport_send_data(DPORT_CMD_RESP_ERRCUSTOM, (uint8*)&result, 4);
                    return 0;
                }

                if ((offset % FLASHBUF_SIZE) != 0)
                {
                    result = 0xa5a5a5a5;
                    dport_send_data(DPORT_CMD_RESP_ERRCUSTOM, (uint8*)&result, 4);
                    return 0;
                }

                memcpy(flashbuf, req->flash.data, length);
                result = dport_hw_iap_write(flashbuf, FLASHBUF_SIZE, offset);
                if (result != 0)
                {
                    dport_send_data(DPORT_CMD_RESP_ERRCUSTOM, (uint8*)&result, 4);
                    return 0;
                }

                resp->flash.offset = req->flash.offset;
                resp->flash.length = req->flash.length;
                dport_send_data(cmd, (uint8*)resp, 8);
                return 0;
            }
        case DPORT_CMD_REQ_FLASH_BUFSIZE:
            resp->flash_bufsize.size = FLASHBUF_SIZE;

            dport_send_data(cmd, (uint8*)resp, sizeof(struct dport_resp_flash_bufsize_t));
            break;
        case DPORT_CMD_REQ_RESET:
            dport_send_data(cmd, NULL, 0);

            /* NOTE insert some pause - let command be processed by host */
            stimer_wait_us(500 * 1000);

            asm volatile (
                "dsb      \r\n"
                "cpsid i  \r\n"
            );
            SCB->AIRCR = (0x05fa << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_SYSRESETREQ_Msk;
            return 1;
//        case DPORT_CMD_REQ_STEP:
//            CoreDebug->DEMCR |= CoreDebug_DEMCR_MON_STEP_Msk;
//            dport_send_data(cmd, NULL, NULL);
//            return 0;
        case DPORT_CMD_REQ_USERF:
            {
                void (*func)(void);

                /* XXX */
                if (req->userf.n >= ((uint32)&_uvect_size / 4))
                {
                    dport_send_data(DPORT_CMD_RESP_ERRCUSTOM, (uint8*)&req->userf.n, 1);
                    return 0;
                }

                dport_send_data(cmd, NULL, 0);

                /* NOTE insert some pause - let command be processed by host */
                stimer_wait_us(200 * 1000);

                func = (void*)&_uvect_start[req->userf.n];

                /* call user-defined function */
                (*func)();
                break;
            }
        default:
            dport_send_data(DPORT_CMD_RESP_CMDUNKNOWN, &cmd, 1);
    }

    return 0;
}

/*
 *
 */
static void dport_send_data(uint8 cmd, uint8 *data, int len)
{
    uint8 cs;
    uint16 dup;

    dport_hw_putchar('%');
    dport_hw_putchar(cmd);

    cs = dport_cs(&cmd, NULL, 0);
    if (data)
    {
        while (len--)
        {
            cs = dport_cs(&cs, data, 1);
            dup = dport_num2dup(*data++);
            dport_hw_putchar(dup & 0xff);
            dport_hw_putchar(dup >> 8);
        }
    }
    dup = dport_num2dup(cs);
    dport_hw_putchar(dup & 0xff);
    dport_hw_putchar(dup >> 8);

    dport_hw_putchar('\r');
    dport_hw_putchar('\n');
}

/*
 *
 */
static uint16 dport_num2dup(uint8 num)
{
    uint16 ret;
    uint8  n;

    ret = 0;
    n   = num & 0x0f;
    if (n < 10)
        ret |= (n + '0');
    else
        ret |= (n + 'A' - 10);

    n   = num >> 4;
    ret <<= 8;
    if (n < 10)
        ret |= (n + '0');
    else
        ret |= (n + 'A' - 10);

    return ret;
}

/*
 *
 */
static uint8 dport_dup2num(uint16 dup)
{
    uint8 ret;
    uint8 n;

    ret = 0;
    n = dup & 0xff;
    if (n < 'A')
        ret |= (n - '0'     ) & 0x0f;
    else
        ret |= (n - 'A' + 10) & 0x0f;

    ret <<= 4;
    n = dup >> 8;
    if (n < 'A')
        ret |= (n - '0'     ) & 0x0f;
    else
        ret |= (n - 'A' + 10) & 0x0f;

    return ret;
}

/*
 *
 */
static uint8 dport_cs(uint8 *cs, uint8 *data, int len)
{
    uint8 ret;

    ret = 0;
    if (cs)
        ret = *cs;
    if (data)
    {
        while (len--)
            ret += *data++;
    }

    return ret & 0xff;
}

///*
// *
// */
//static void dport_send_str(char *st)
//{
//    if (!st)
//        return;
//    for (; *st;st++)
//        dport_hw_putchar(*st);
//}
//
