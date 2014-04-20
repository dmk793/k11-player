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
 * vs1053b control functions
 */
#include <debug.h>
#include <os.h>
#include <stimer.h>
#include <string.h>
#include "vs1053b.h"
#include "vs1053b_hw.h"
#include "../player/player.h"

#define DEBUG_VS1053B

#ifdef DEBUG_VS1053B
    #define DPRINT(fmt, ...)  dprint(fmt, __VA_ARGS__)
#else
    #define DPRINT(fmt, ...)
#endif

/**************************
 * VS1053B registers
 **************************/
#define SCI_REG_MODE          0x00
#define SCI_REG_STATUS        0x01
#define SCI_REG_CLOCKF        0x03
#define SCI_REG_DECODE_TIME   0x04
#define SCI_REG_WRAM          0x06
#define SCI_REG_WRAMADDR      0x07
#define SCI_REG_HDAT0         0x08
#define SCI_REG_HDAT1         0x09
#define SCI_REG_VOL           0x0b

//#define SCI_RAM_DREQ          0xC012
#define SCI_RAM_PLAYSPEED     0x1e04
#define SCI_RAM_ENDFILLBYTE   0x1e06

//#define SCI_REG_STATUS_SS_REFERENCE_SEL (1 << 0)
//
//#define SCI_REG_MODE_SM_RESET           (1 << 2)
//#define SCI_REG_MODE_SM_CANCEL          (1 << 3)

/**************************/

#pragma pack(push, 1)
struct vs1053b_state_t {
    struct {
        union {
            struct {
                uint8 diff              :1; /*  0, differential */
                uint8 layer12           :1; /*  1, not used in VS8053 */
                uint8 reset             :1; /*  2, soft reset */
                uint8 cancel            :1; /*  3, cancel decoding curent file */
                uint8 earspeaker        :1; /*  4, earspeaker low setting */
                uint8 tests             :1; /*  5, allow SDI tests */
                uint8 stream            :1; /*  6, stream mode */
                uint8 earspeaker_hi     :1; /*  7, earspeaker high setting */
                uint8 dact              :1; /*  8, DCLK active edge */
                uint8 sdiord            :1; /*  9, SDI bit order */
                uint8 sdishare          :1; /* 10, share SPI chip select */
                uint8 sdinew            :1; /* 11, VS1002 native SPI modes */
                uint8 adpcm             :1; /* 12, PCM/ADPCM recording active */
                uint8 reserved          :1; /* 13, */
                uint8 line1             :1; /* 14, MIC/LINE1 selector */
                uint8 clk_range         :1; /* 15, input clock range */
            };
            uint16 value;
        } mode;
        union {
            struct {
                uint8 reference_sel : 1; /*       0,   reference voltage selection */
                uint8 ad_clock      : 1; /*       1,   AD clock select */
                uint8 apdown1       : 1; /*       2,   analog internal powerdown */
                uint8 apdown2       : 1; /*       3,   analog driver powerdown */
                uint8 ver           : 4; /*  4 -  7,   version */
                uint8 reserved      : 2; /*  8 -  9 */
                uint8 vcm_disable   : 1; /*      10    GBUF overload detection */
                uint8 vcm_overload  : 1; /*      11    GBUF overload indicator */
                uint8 swing         : 3; /* 12 - 14    set swing */
                uint8 do_not_jump   : 1; /*      15    header in decode, do not fast forward/rewind */
            };
            uint16 value;
        } status;
        union {
            struct {
                uint16 freq :  11; /*   0 - 10, clock frequency */
                uint16 add  :  2;  /*  11 - 12, allowed multiplier addition */
#define SCI_CLOKF_MULT_1X       0
#define SCI_CLOKF_MULT_2X       1
#define SCI_CLOKF_MULT_4X       5
                uint16 mult :  3;  /*  13 - 15, clock multiplier */
            };
            uint16 value;
        } clockf;
    } reg;
};
#pragma pack(pop)

struct vs1053b_state_t vs1053b;

/*
 * NOTE called from player_task()
 */
int vs1053b_init()
{
    if (!vs1053b_hw_init())
    {
        DEBUG_EMSG("HW init error");
        return 0;
    }
    
    /* read STATUS */
    {
        vs1053b.reg.status.value = vs1053b_hw_readsci(SCI_REG_STATUS);
        vs1053b.reg.mode.value   = vs1053b_hw_readsci(SCI_REG_MODE);
        DEBUG_IMSGF("STATUS", "2xn", vs1053b.reg.status.value);
        DEBUG_IMSGF("MODE  ", "2xn", vs1053b.reg.mode.value);

        /* SS_VER (bits 7:4) should be 4 for vs1053 and vs8053 */
        if (vs1053b.reg.status.ver != 4)
        {
            DEBUG_EMSG("SS_VER != 4");
            return 0;
        }
    }

    /* set reference voltage to 1.65V */
    vs1053b.reg.status.reference_sel = 1;
    vs1053b_hw_writesci(SCI_REG_STATUS, vs1053b.reg.status.value);

    /* decrease volume */
//    vs1053b_hw_writesci(SCI_REG_VOL, 0x4141);
//    vs1053b_hw_writesci(SCI_REG_VOL, 0x2020);

    /* set clock */
    {
        int n;

        vs1053b.reg.clockf.value = vs1053b_hw_readsci(SCI_REG_CLOCKF);

#define RETRY_COUNT    5
        n = RETRY_COUNT;
        while (n--)
        {
            vs1053b.reg.clockf.mult = SCI_CLOKF_MULT_4X;
            vs1053b_hw_writesci(SCI_REG_CLOCKF, vs1053b.reg.clockf.value);

            vs1053b.reg.clockf.value = vs1053b_hw_readsci(SCI_REG_CLOCKF);

            DEBUG_IMSGF("CLOCKF", "2xn", vs1053b.reg.clockf.value);
            if (vs1053b.reg.clockf.mult != SCI_CLOKF_MULT_4X)
            {
                DEBUG_EMSG("can not set CLOCKF MULT");
                os_wait_ms(10);
            } else {
                break;
            }
        }
        if (n == 0)
            return 0;
    }

    return 1;
}

/*
 * read ram location of vs1053b
 */
static uint16 vs1053b_rram(uint16 addr)
{
    vs1053b_hw_writesci(SCI_REG_WRAMADDR, addr);
    return vs1053b_hw_readsci(SCI_REG_WRAM);
}

/*
 * write ram location of vs1053b
 */
static void vs1053b_wram(uint16 addr, uint16 value)
{
    vs1053b_hw_writesci(SCI_REG_WRAMADDR, addr);
    vs1053b_hw_writesci(SCI_REG_WRAM, value);
}

/*
 * RETURN
 *     1 if dreq is ready to transfer data
 *
 */
inline int vs1053b_check_dreq()
{
    return vs1053b_hw_check_dreq();
}

/*
 *
 */
inline void vs1053b_wait_dreq()
{
    vs1053b_hw_wait_dreq();
}

/*
 *
 */
void vs1053b_set_hi_fclk()
{
    vs1053b_hw_set_hi_fclk();
}

/*
 *
 */
void vs1053b_set_low_fclk()
{
    vs1053b_hw_set_low_fclk();
}

/*
 *
 */
inline void vs1053b_writesdi(uint8 *data, int len)
{
    vs1053b_hw_writesdi(data, len);
}

/*
 * XXX not match documentation
 */
void vs1053b_cancel()
{
    uint16 efillbyte;
    uint32 to;
#define CANCEL_BUFSIZE    32
    uint8  cbuf[CANCEL_BUFSIZE];
    int    bcnt;

    vs1053b_set_low_fclk();

    /* read endFillByte */
    efillbyte = vs1053b_rram(SCI_RAM_ENDFILLBYTE);
    DEBUG_IMSGF("endfillbyte", "2xn", efillbyte);
    memset(cbuf, efillbyte, CANCEL_BUFSIZE);

    /* set SM_CANCEL */
    vs1053b.reg.mode.cancel = 1;
    vs1053b_hw_writesci(SCI_REG_MODE, vs1053b.reg.mode.value);

    stimer_settime(&to);
    bcnt = 0;
#define CANCEL_TIMEOUT    1000
    while (stimer_deltatime(to) < CANCEL_TIMEOUT && bcnt < 2052)
    {
        vs1053b_set_hi_fclk();
        /* write 32 bytes of data */
        if (!vs1053b_check_dreq())
        {
            os_wait(1);
            continue;
        }

        vs1053b_hw_writesdi(cbuf, CANCEL_BUFSIZE);
        bcnt += CANCEL_BUFSIZE;

        vs1053b_set_low_fclk();

        /* check SM_CANCEL */
        vs1053b.reg.mode.value = vs1053b_hw_readsci(SCI_REG_MODE);
        if (!vs1053b.reg.mode.cancel)
        {
            uint16 hdat0, hdat1;

            hdat0 = vs1053b_hw_readsci(SCI_REG_HDAT0);
            hdat1 = vs1053b_hw_readsci(SCI_REG_HDAT1);

            /* check HDAT0 and HDAT1 */
            if (hdat0 != 0 || hdat1 != 0)
            {
                DEBUG_WMSGF("still decoding", "2x_2xn", hdat0, hdat1);
            } else {
                DEBUG_IMSG("cancel success");
            }
            return;
        }
    }
    DEBUG_WMSG("cancel timeout/error");

    vs1053b_set_low_fclk();

    /* do software reset */
    vs1053b.reg.mode.reset  = 1;
    vs1053b.reg.mode.cancel = 0;
    vs1053b_hw_writesci(SCI_REG_MODE, vs1053b.reg.mode.value);
    vs1053b.reg.mode.reset  = 0;
    os_wait_ms(100);
}

/*
 * return decode time
 */
int vs1053b_get_decode_time()
{
    return vs1053b_hw_readsci(SCI_REG_DECODE_TIME);
}

/*
 * set decode time
 */
void vs1053b_set_decode_time(uint16 time)
{
    vs1053b_hw_writesci(SCI_REG_DECODE_TIME, time);
    vs1053b_hw_writesci(SCI_REG_DECODE_TIME, time);
}


/*
 * set play speed
 */
void vs1053b_set_play_speed(uint16 value)
{
    vs1053b_wram(SCI_RAM_PLAYSPEED, value);
}

/*
 * set volume
 *
 * ARGS
 *     value    volume level to set in percents
 * 
 * RETURN
 *     value of current volume level of decoder in percents
 *
 */
int
vs1053b_set_volume(int value)
{
    uint16 writeval;
    uint16 readval;
//#define VOLUME_MIN    0xfe
#define VOLUME_MIN    0x60
#define VOLUME_MAX    0x00

    if (value < 0) value = 0;
    if (value > 100) value = 100;

    writeval  = VOLUME_MAX + ((VOLUME_MIN - VOLUME_MAX) * (100 - value) / 100);
    writeval |= (writeval << 8);
    vs1053b_hw_writesci(SCI_REG_VOL, writeval);

    readval = vs1053b_hw_readsci(SCI_REG_VOL);
    if (readval != writeval)
        DEBUG_EMSG("volume set failed");

    readval &= 0xff;

    if (readval > VOLUME_MIN)
        return 0;
    else
        return (100 - ((readval - VOLUME_MAX) * 100 / (VOLUME_MIN - VOLUME_MAX)));
}

