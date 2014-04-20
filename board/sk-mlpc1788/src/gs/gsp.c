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

#include <clk_cfg.h>
#include <LPC177x_8x.h>
#include <LPC177x_8x_bits.h>
#include <stimer.h>
#include <debug.h>
#include <gpio.h>
#include <string.h> /* for memcpy */
#include <os.h>
#include "gsp.h"
#include "../dma.h"
#include "../irqp.h"

#define DEBUG_GSP

#ifdef DEBUG_GSP
    #define DPRINT(fmt, ...) dprint(fmt, __VA_ARGS__)
#else
    #define DPRINT(fmt, ...)
#endif

static const struct gpio_t lcd_reset = {LPC_GPIO0, (1 << 15)};

#define LCD_CLKRATE   (12 * 1000 * 1000) /* Hz */
//#define LCD_CLKRATE   (6 * 1000 * 1000) /* Hz */
#define LCD_CLKDIV    (CLK_CCLK / LCD_CLKRATE)
#if (LCD_CLKDIV > 31)
    #error LCD clock divider too high
#endif
#define LCD_HSW       41 /* clocks */
#define LCD_HFP        2 /* clocks */
#define LCD_HBP        2 /* clocks */
#define LCD_VSW       10 /* clocks */
#define LCD_VFP        2 /* lines  */
#define LCD_VBP        2 /* lines  */

#define GSP_HW_RESET_SET()         {gpio_drive(&lcd_reset, 0);}
#define GSP_HW_RESET_RELEASE()     {gpio_drive(&lcd_reset, 1);}

struct gs_private_t gsp;

/*
 * initialize hardware
 *
 * MI0430FT (SK-MI0430FT-Plug module)
 * 480x272, 24 bpp
 */
void gsp_hw_init()
{
//    /* NOTE initialize DMA */
//    dma_init();

    /*
     * configure pins
     *
     * P2[0]    LCD_PWR    (FUNC 7) -> NC ?
     * P2[1]    LCD_LE     (FUNC 7) -> NC   XXX backlight control?
     * P2[2]    LCD_DCLK   (FUNC 7) -> PCLK
     * P2[3]    LCD_FP     (FUNC 7) -> VS
     * P2[4]    LCD_ENAB_M (FUNC 7) -> DE
     * P2[5]    LCD_LP     (FUNC 7) -> HS
     *
     * P0[4]    LCD_VD0    (FUNC 7) -> R0
     * P0[5]    LCD_VD1    (FUNC 7) -> R1
     * P4[28]   LCD_VD2    (FUNC 7) -> R2
     * P4[29]   LCD_VD3    (FUNC 7) -> R3
     * P2[6]    LCD_VD4    (FUNC 7) -> R4
     * P2[7]    LCD_VD5    (FUNC 7) -> R5
     * P2[8]    LCD_VD6    (FUNC 7) -> R6
     * P2[9]    LCD_VD7    (FUNC 7) -> R7
     *
     * P0[6]    LCD_VD8    (FUNC 7) -> G0
     * P0[7]    LCD_VD9    (FUNC 7) -> G1
     * P1[20]   LCD_VD10   (FUNC 7) -> G2
     * P1[21]   LCD_VD11   (FUNC 7) -> G3
     * P1[22]   LCD_VD12   (FUNC 7) -> G4
     * P1[23]   LCD_VD13   (FUNC 7) -> G5
     * P1[24]   LCD_VD14   (FUNC 7) -> G6
     * P1[25]   LCD_VD15   (FUNC 7) -> G7
     *
     * P0[8]    LCD_VD16   (FUNC 7) -> B0
     * P0[9]    LCD_VD17   (FUNC 7) -> B1
     * P2[12]   LCD_VD18   (FUNC 7) -> B2
     * P2[13]   LCD_VD19   (FUNC 7) -> B3
     * P1[26]   LCD_VD20   (FUNC 7) -> B4
     * P1[27]   LCD_VD21   (FUNC 7) -> B5
     * P1[28]   LCD_VD22   (FUNC 7) -> B6
     * P1[29]   LCD_VD23   (FUNC 7) -> B7
     *
     * P0[15]                       -> Reset
     */
    gpio_setdir(&lcd_reset, GPIO_DIR_OUTPUT);
    GSP_HW_RESET_SET();

    LPC_IOCON_FUNC_SET(P2_2,  7); /*   LCD_DCLK    -> PCLK */
    LPC_IOCON_FUNC_SET(P2_3,  7); /*   LCD_FP      -> VS */
    LPC_IOCON_FUNC_SET(P2_4,  7); /*   LCD_ENAB_M  -> DE */
    LPC_IOCON_FUNC_SET(P2_5,  7); /*   LCD_LP      -> HS */

    LPC_IOCON_FUNC_SET(P0_4,  7); /*   LCD_VD0    -> R0 */
    LPC_IOCON_FUNC_SET(P0_5,  7); /*   LCD_VD1    -> R1 */
    LPC_IOCON_FUNC_SET(P4_28, 7); /*   LCD_VD2    -> R2 */
    LPC_IOCON_FUNC_SET(P4_29, 7); /*   LCD_VD3    -> R3 */
    LPC_IOCON_FUNC_SET(P2_6,  7); /*   LCD_VD4    -> R4 */
    LPC_IOCON_FUNC_SET(P2_7,  7); /*   LCD_VD5    -> R5 */
    LPC_IOCON_FUNC_SET(P2_8,  7); /*   LCD_VD6    -> R6 */
    LPC_IOCON_FUNC_SET(P2_9,  7); /*   LCD_VD7    -> R7 */

    LPC_IOCON_FUNC_SET(P0_6,  7); /*   LCD_VD8    -> G0 */
    LPC_IOCON_FUNC_SET(P0_7,  7); /*   LCD_VD9    -> G1 */
    LPC_IOCON_FUNC_SET(P1_20, 7); /*   LCD_VD10   -> G2 */
    LPC_IOCON_FUNC_SET(P1_21, 7); /*   LCD_VD11   -> G3 */
    LPC_IOCON_FUNC_SET(P1_22, 7); /*   LCD_VD12   -> G4 */
    LPC_IOCON_FUNC_SET(P1_23, 7); /*   LCD_VD13   -> G5 */
    LPC_IOCON_FUNC_SET(P1_24, 7); /*   LCD_VD14   -> G6 */
    LPC_IOCON_FUNC_SET(P1_25, 7); /*   LCD_VD15   -> G7 */

    LPC_IOCON_FUNC_SET(P0_8,  7); /*   LCD_VD16   -> B0 */
    LPC_IOCON_FUNC_SET(P0_9,  7); /*   LCD_VD17   -> B1 */
    LPC_IOCON_FUNC_SET(P2_12, 7); /*   LCD_VD18   -> B2 */
    LPC_IOCON_FUNC_SET(P2_13, 7); /*   LCD_VD19   -> B3 */
    LPC_IOCON_FUNC_SET(P1_26, 7); /*   LCD_VD20   -> B4 */
    LPC_IOCON_FUNC_SET(P1_27, 7); /*   LCD_VD21   -> B5 */
    LPC_IOCON_FUNC_SET(P1_28, 7); /*   LCD_VD22   -> B6 */
    LPC_IOCON_FUNC_SET(P1_29, 7); /*   LCD_VD23   -> B7 */

    LPC_SC->PCONP |= PCONP_PCLCD;
    LPC_LCD->CTRL = 0; /* disable LCD controller */

    LPC_SC->LCD_CFG = (LCD_CLKDIV - 1);

#define LCD_POL_CLKSEL_CCLK        (0 << 5)  /* clock source is CCLK */
#define LCD_POL_CLKSEL_CLKIN       (1 << 5)  /* clock source is LCD_CLKIN */
#define LCD_POL_IVS_FP_ACTIVE_HIGH (0 << 11) /* frame pulse (vsync) active HIGH */
#define LCD_POL_IVS_FP_ACTIVE_LOW  (1 << 11) /* frame pulse (vsync) active LOW */
#define LCD_POL_IHS_LP_ACTIVE_HIGH (0 << 12) /* line pulse (hsync) active HIGH */
#define LCD_POL_IHS_LP_ACTIVE_LOW  (1 << 12) /* line pulse (hsync) active LOW */
#define LCD_POL_IPC_CHANGE_RISING  (0 << 13) /* data is driven on the LCD lines on the RISING edge */
#define LCD_POL_IPC_CHANGE_FALLING (1 << 13) /* data is driven on the LCD lines on the FALLING edge */
#define LCD_POL_IOE_ACTIVE_HIGH    (0 << 14) /* LCD_ENAB_M pin is active HIGH */
#define LCD_POL_IOE_ACTIVE_LOW     (1 << 14) /* LCD_ENAB_M pin is active HIGH  */
#define LCD_POL_BCD                (1 << 26) /* bypass pixel clock divider */
    LPC_LCD->POL = 
        LCD_POL_CLKSEL_CCLK |
        LCD_POL_IVS_FP_ACTIVE_LOW |
        LCD_POL_IHS_LP_ACTIVE_LOW |
        LCD_POL_BCD |
//        LCD_POL_IPC_CHANGE_FALLING |
        ((GS_RES_HORIZONTAL - 1) << 16); /* CPL, clock per line. */

    LPC_LCD->TIMH =
        ((GS_RES_HORIZONTAL / 16 - 1) << 2) | /* PPL, pixel per line */
        ((LCD_HSW - 1) << 8)  |               /* HSW, horizontal synchonization pulse width */
        ((LCD_HFP - 1) << 16) |               /* HFP, horizontal front porch */
        ((LCD_HBP - 1) << 24);                /* HBP, horizontal back porch */

    LPC_LCD->TIMV = 
        (GS_RES_VERTICAL - 1) | /* Lines per panel. This is the number of active lines per screen */
        ((LCD_VSW - 1) << 10) | /* VSW, vertical synchronization pulse width */
        ((LCD_VFP - 1) << 16) | /* VFP, vertical front porch */
        ((LCD_VBP - 1) << 24);  /* VBP, vertical back porch */

    gsp_set_active_buffer(gs_active_buf); /* NOTE gsp.mem should be initialized */
#define LCD_CTRL_LCDEN          (1 << 0)    /* LCD enable control bit */
#define LCD_CTRL_BPP_1BPP       (0 << 1)    /* LCD bits per pixel, 1 bpp */
#define LCD_CTRL_BPP_2BPP       (1 << 1)    /* 2 bpp */
#define LCD_CTRL_BPP_4BPP       (2 << 1)    /* 4 bpp */
#define LCD_CTRL_BPP_8BPP       (3 << 1)    /* 8 bpp */
#define LCD_CTRL_BPP_16BPP      (4 << 1)    /* 16 bpp */
#define LCD_CTRL_BPP_24BPP      (5 << 1)    /* 24 bpp (TFT panel only) */
#define LCD_CTRL_BPP_16BPP_565  (6 << 1)    /* 16 bpp, 5:6:5 mode */
#define LCD_CTRL_BPP_12BPP_444  (7 << 1)    /* 12 bpp, 4:4:4 mode */
#define LCD_CTRL_LCDBW_COLOR    (0 << 4)    /* STN LCD is color */
#define LCD_CTRL_LCDBW_MONOCH   (1 << 4)    /* STN LCD is monochrome */
#define LCD_CTRL_LCDTFT_STN     (0 << 5)    /* LCD is STN. Use grayscaller */
#define LCD_CTRL_LCDTFT_TFT     (1 << 5)    /* LCD is TFT. Do not use grayscaller */
#define LCD_CTRL_LCDMONO8_4BIT  (0 << 6)    /* monochrome LCD uses a 4-bit interface */
#define LCD_CTRL_LCDMONO8_8BIT  (1 << 6)    /* monochrome LCD uses a 8-bit interface */
#define LCD_CTRL_LCDDUAL_SINGLE (0 << 7)    /* STN LCD interface is single-panel */
#define LCD_CTRL_LCDDUAL_DUAL   (1 << 7)    /* STN LCD interface is dual-panel */
#define LCD_CTRL_BGR_RGB        (0 << 8)    /* RGB: normal output */
#define LCD_CTRL_BGR_BGR        (1 << 8)    /* BGR: red and blue swapped */
#define LCD_CTRL_BEPO_BYTE_LE   (0 << 9)    /* little endian byte order in memory */
#define LCD_CTRL_BEPO_BYTE_BE   (1 << 9)    /* big endian byte order in memory */
#define LCD_CTRL_BEPO_PIXEL_LE  (0 << 10)   /* little endian ordering within byte */
#define LCD_CTRL_BEPO_PIXEL_BE  (1 << 10)   /* big endian ordering within byte */
#define LCD_CTRL_LCDPWR         (1 << 11)   /* LCD power enable */
#define LCD_CTRL_LCDVCOMP_VS    (0 << 12)   /* LCD Vertical Compare interrupt. Start of vertical sync. */
#define LCD_CTRL_LCDVCOMP_BP    (1 << 12)   /* start of back porch */
#define LCD_CTRL_LCDVCOMP_AV    (2 << 12)   /* start of active video  */
#define LCD_CTRL_LCDVCOMP_FP    (3 << 12)   /* start of front porch */
#define LCD_CTRL_WATERMARK_4    (0 << 16)   /* LCD DMA request is generated when either of the DMA FIFOs have four or more empty locations.  */
#define LCD_CTRL_WATERMARK_8    (1 << 16)   /* LCD DMA request is generated when either of the DMA FIFOs have eight or more empty locations. */
    LPC_LCD->CTRL =
        LCD_CTRL_BPP_24BPP |
        LCD_CTRL_LCDTFT_TFT;        
    LPC_LCD->CTRL |= LCD_CTRL_LCDEN; /* enable LCD controller */
    os_wait_ms(10);
    GSP_HW_RESET_RELEASE();
    LPC_LCD->CTRL |= LCD_CTRL_LCDPWR; /* set LCD_POWER, LCD_VD[23:0] go to active state */
    os_wait_ms(20);

#define LCD_INTMSK_LNBUI   (1 << 2) /* next base address update interrupt */
    LPC_LCD->INTMSK = LCD_INTMSK_LNBUI;
    NVIC_SetPriority(LCD_IRQn, LCD_IRQP);
#ifndef GS_SINGLE_WINDOW
    NVIC_EnableIRQ(LCD_IRQn);
#endif
}

/*
 *
 */
void gsp_set_active_buffer(union gs_pixel_t *mem)
{
    LPC_LCD->UPBASE = ((uint32)mem) & (~0x0000007);
    LPC_LCD->LPBASE = ((uint32)mem) & (~0x0000007);
}

/*
 *
 */
void* gsp_malloc(int size)
{
    return os_malloc(size);
}

/*
 *
 */
void gsp_free(void *p)
{
    os_mfree(p);
}

/*
 *
 */
void gsp_memcpy(void *dst, void *src, int size)
{
    memcpy(dst, src, size);
}

/*
 *
 */
void gsp_memset(void *mem, int pat, int size)
{
    memset(mem, pat, size);
}

/*
 *
 */
void gsp_mutex_lock()
{
    os_mutex_lock(&gsp.mutex, GSP_MUTEX_1, OS_FLAG_NONE, OS_WAIT_FOREVER);
}

/*
 *
 */
void gsp_mutex_unlock()
{
    os_mutex_unlock_ns(&gsp.mutex, GSP_MUTEX_1);
}

/*
 *
 */
void gsp_enter_crit()
{
    os_disable_irq();
}

/*
 *
 */
void gsp_exit_crit()
{
    os_enable_irq();
}

/*
 *
 */
void gsp_win_map(struct gs_win_t *win, union gs_pixel_t *dst)
{
    void *src;
    uint32 srcwidth;
    uint32 lines;

    if (!win)
        return;

    dst      += win->y * GS_RES_HORIZONTAL + win->x;
    src      = win->mem;
    srcwidth = (win->x + win->width)  > GS_RES_HORIZONTAL ? (GS_RES_HORIZONTAL - win->x) : win->width;
    srcwidth *= GS_PIXEL_DEPTH;
    lines    = (win->y + win->height) > GS_RES_VERTICAL ? (GS_RES_VERTICAL - win->y) : win->height;

    dma_copy_window(dst, GS_RES_HORIZONTAL * GS_PIXEL_DEPTH,
        src, srcwidth, win->width * GS_PIXEL_DEPTH, lines);
}

/*
 *
 */
void gsp_image_map(struct gs_win_t *win, int x, int y, uint8 *idata, int iwidth, int iheight)
{
    void *src;
    void *dst;
    uint32 srcwidth;

    if (!win || !idata)
        return;

    if ((x + iwidth)  > win->width)
        return;
    if ((y + iheight) > win->height)
        return;

    dst      = &win->mem[y * win->width + x];
    src      = idata;
    srcwidth = iwidth * GS_PIXEL_DEPTH;

    dma_copy_window(dst, win->width * GS_PIXEL_DEPTH,
        src, srcwidth, srcwidth, iheight);
}

/*
 *
 */
void gsp_win_map2(struct gs_win_t *win, union gs_pixel_t *dst, struct gs_pane_t *pane)
{
    void *src;
    uint32 srcwidth;
    uint32 lines;
    int sx, sy;

    if (!win)
        return;


    sx = pane->x - win->x; /* start X coordinate relative to window */
    sy = pane->y - win->y; /* start Y coordinate relative to window */

    /* sanity check */
    if (!pane->width || (pane->width + sx) > win->width)
        return;
    if (!pane->height || (pane->height + sy) > win->height)
        return;
    if (pane->x < win->x)
        return;
    if (pane->y < win->y)
        return;

    dst      += pane->y * GS_RES_HORIZONTAL + pane->x;
    src      = &win->mem[win->width * sy + sx];

    srcwidth = pane->width;
    if ((srcwidth + pane->x) > GS_RES_HORIZONTAL)
        srcwidth = GS_RES_HORIZONTAL - pane->x;
    srcwidth *= GS_PIXEL_DEPTH;

    lines = pane->height;
    if ((lines + pane->y) > GS_RES_VERTICAL)
        lines = GS_RES_VERTICAL - pane->y;

    dma_copy_window(dst, GS_RES_HORIZONTAL * GS_PIXEL_DEPTH,
        src, srcwidth, win->width * GS_PIXEL_DEPTH, lines);
}

#ifdef GS_SINGLE_WINDOW
void gsp_win_refresh(struct gs_win_t *win)
{
    gsp_mutex_lock();
    {
        gsp_wait_vsync();
        gs_switch_abuf();
        single_window.mem = gs_inactive_buf;

        /* NOTE copy active buffer (inactive recently) to inactive buffer (active recently) */
        dma_copy_window(gs_inactive_buf, GS_RES_HORIZONTAL * GS_PIXEL_DEPTH,
            gs_active_buf,
            GS_RES_HORIZONTAL * GS_PIXEL_DEPTH,
            GS_RES_HORIZONTAL * GS_PIXEL_DEPTH, GS_RES_VERTICAL);
    }
    gsp_mutex_unlock();
}

/*
 *
 */
void gsp_win_refresh_widgets(struct gs_win_t *win)
{
    struct gs_widget_t *wd;
    void *src, *dst;
    int srclen, srcwidth, lines;

    gsp_mutex_lock();
    {
        gsp_wait_vsync();
        gs_switch_abuf();
        single_window.mem = gs_inactive_buf;

        for (wd = win->widgets; wd != NULL; wd = wd->next)
        {
            if (wd->map)
            {
                wd->map--;

                /* 
                 * Widget was drawn on recently inactive buffer.
                 * Now buffer is active. Copy widget area to newly inactive
                 * buffer.
                 */

                /* NOTE gs_mem is in (union gs_pixel_t) format */
                dst = &gs_inactive_buf[wd->y0 * GS_RES_HORIZONTAL + wd->x0];
                src = &gs_active_buf[wd->y0 * GS_RES_HORIZONTAL + wd->x0];

                /* sanity check */
                if (wd->x0 > wd->x1 || wd->y0 > wd->y1)
                {
                    DEBUG_EMSG("invalid widget area");
                    continue; 
                }

                srclen   = (wd->x1 - wd->x0 + 1) * GS_PIXEL_DEPTH;
                srcwidth = GS_RES_HORIZONTAL * GS_PIXEL_DEPTH;
                lines    = (wd->y1 - wd->y0 + 1);

                dma_copy_window(dst, GS_RES_HORIZONTAL * GS_PIXEL_DEPTH,
                    src, srclen, srcwidth, lines);
            }
        }
    }
    gsp_mutex_unlock();
}

/*
 *
 */
void gsp_wait_vsync()
{
    LPC_LCD->INTCLR = LCD_INTMSK_LNBUI;
    NVIC_EnableIRQ(LCD_IRQn);
    os_event_wait(&gsp.event, GSP_EVENT_MASK_VSYNC, OS_FLAG_CLEAR, OS_WAIT_FOREVER);
}
#else
/*
 *
 */
void gsp_win_refresh(struct gs_win_t *win)
{
    if (!win)
        return;

    gsp_mutex_lock();
    win->map = GS_MEM_BUFNUM;
    gsp_mutex_unlock();
    os_event_wait(&win->events, GS_WIN_EVENT_MASK_REFRESH, OS_FLAG_CLEAR, OS_WAIT_FOREVER);
}

/*
 *
 */
void gsp_win_refresh_finish(struct gs_win_t *win)
{
    if (!win)
        return;

    os_event_raise(&win->events, GS_WIN_EVENT_MASK_REFRESH);
}

/*
 *
 */
void gsp_wait_vsync()
{
    os_event_wait(&gsp.event, GSP_EVENT_MASK_VSYNC, OS_FLAG_CLEAR, OS_WAIT_FOREVER);
}

#endif /* GS_SINGLE_WINDOW */

/*
 * IRQ Handler
 */
void LCD_Handler(void)
{
    if (LPC_LCD->INTSTAT & LCD_INTMSK_LNBUI)
    {
        LPC_LCD->INTCLR = LCD_INTMSK_LNBUI;
#ifdef GS_SINGLE_WINDOW
        NVIC_DisableIRQ(LCD_IRQn);
#endif
        os_event_raise(&gsp.event, GSP_EVENT_MASK_VSYNC);
        return;
    }
}

/*
 *
 */
void gsp_init_end()
{
    gsp_enter_crit();
    gsp.initialized = 1;
    gsp_exit_crit();
}

/*
 *
 */
int gsp_initialized()
{
    int ret;
    gsp_enter_crit();
    ret = gsp.initialized;
    gsp_exit_crit();
    return ret;
}

