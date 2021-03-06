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
 * SD card functions
 *
 * TODO
 *     * valid retry counters based on clock frequency
 *     * card detect
 *     * SC card support
 *     ? XC card support
 */
#include <LPC177x_8x.h>
#include <debug.h>
#include <string.h>
#include <os.h>
#include "sdcard.h" 
#include "sdcard_hw.h"
#include "../fat_io_lib/fat_filelib.h"

#define DEBUG_SDCARD

#ifdef DEBUG_SDCARD
    #define DPRINT(fmt, ...) dprint(fmt, __VA_ARGS__)
#else
    #define DPRINT(fmt, ...)
#endif

int sdcard_start();
static int sdcard_cmd8_send();
static int sdcard_cmd55_send();
static int sdcard_acmd41_send();
static int sdcard_cmd2_send();
static int sdcard_cmd3_send();
static int sdcard_cmd7_send();
static int sdcard_cmd9_send();
static int sdcard_cmd12_send();
static int sdcard_acmd6_send();
static int sdcard_checkstatus();
static int sdcard_stop_transmission();
static int sdcard_cmd_read_block(uint32 baddr);
static int sdcard_read_block(uint32 bnum, uint8 *data);

struct card_state_t cardstate;

/*
 * perform initialization procedures when card was inserted
 *
 * RETURN
 *     type of detected card
 */
int sdcard_start()
{
    int ctype;
    DPRINT("sn", "Init SD card");

    /* reset RCA */
    cardstate.rca  = 0;
    ctype = SD_CARDTYPE_UNUSABLE;

    sdcard_hw_set_lo_clk();

    /* repeat command 3 times, for safe */
    sdcard_hw_send_cmd(SD_CMD0_GO_IDLE_STATE, SD_CMDFLAG_NO_RESPONSE, NULL, NULL);
    sdcard_hw_send_cmd(SD_CMD0_GO_IDLE_STATE, SD_CMDFLAG_NO_RESPONSE, NULL, NULL);
    sdcard_hw_send_cmd(SD_CMD0_GO_IDLE_STATE, SD_CMDFLAG_NO_RESPONSE, NULL, NULL);

    if (sdcard_cmd8_send())
    {
        DPRINT("tsn", "Ver2.00 or later SD card");
    } else {
        DPRINT("sn", "(E) CMD8 fail (voltage mismatch or Ver1.X SD card)");
        DPRINT("sn", "(W) Ver1.X card support not implemented yet");
        return SD_CARDTYPE_UNUSABLE;
    }

    ctype = sdcard_acmd41_send();
    if (ctype != SD_CARDTYPE_UNUSABLE)
    {
        DPRINT("tsn", ctype == SD_CARDTYPE_SD_V200_SC ? "SDSC" : "SDHC/SDXC");
        if (ctype == SD_CARDTYPE_SD_V200_SC)
        {
            DPRINT("sn", "(W) SDSC card support not implemented yet");
            return SD_CARDTYPE_UNUSABLE;
        }
    } else {
        DPRINT("sn", "(E) ACMD41 failed");
        return SD_CARDTYPE_UNUSABLE;
    }
    /* NOTE S18R, S18A not used */

    /* CMD2 (ALL_SEND_CID) */
    if (!sdcard_cmd2_send())
    {
        DPRINT("sn", "(E) CMD2 failed");
        return SD_CARDTYPE_UNUSABLE;
    }

    /* CMD3 (SEND_RELATIVE_ADDR) */
    if (!sdcard_cmd3_send())
    {
        DPRINT("sn", "(E) CMD3 failed");
        return SD_CARDTYPE_UNUSABLE;
    }

    /* CMD9 (SEND_CSD) */
    if (!sdcard_cmd9_send())
    {
        DPRINT("sn", "(E) CMD9 failed");
        return SD_CARDTYPE_UNUSABLE;
    }

    /* CMD7 (SELECT/DESELECT_CARD) */
    if (!sdcard_cmd7_send())
    {
        DPRINT("sn", "(E) CMD7 failed");
        return SD_CARDTYPE_UNUSABLE;
    }

    sdcard_hw_set_hi_clk();

    /* ACMD6 (SET_BUS_WIDTH) */
    if (!sdcard_acmd6_send())
    {
        DPRINT("sn", "(E) ACMD6 failed");
        return SD_CARDTYPE_UNUSABLE;
    }

//    {
//        uint8 block[513];
//        block[512] = 0xa1;
//        uint32 to;
//        int i;
//
//        for (i = 0; i < 1; i++)
//        {
//            stimer_settime(&to);
//            sdcard_read_block(17858 + i, block);
//            to = stimer_deltatime(to);
//            dprint("s4dn", "time ", to);
//
//            dprint("s", "block:");
//            dprint("lb", 512, block);
//            dprint("s1xn", "byte = ", block[512]);
//        }
//    }

//    {
//        if (!sdcard_checkstatus())
//        {
//            DPRINT("sn", "(E) check status failed");
//            return SD_CARDTYPE_UNUSABLE;
//        }
//
//
//        if (!sdcard_stop_transmission())
//        {
//            DPRINT("sn", "(E) stop transmission failed");
//            return SD_CARDTYPE_UNUSABLE;
//        }
//    }

    return ctype;
}

/*
 * read block of data from card
 *
 * RETURN
 *    1 on success, 0 otherwise
 */
int sdcard_read(uint32 bnum, uint8 *data)
{
//    int ctype;
//
//    ctype = cardstate.card_type;
//
//    if (ctype == SD_CARDTYPE_UNUSABLE)
//        return 0;

    return sdcard_read_block(bnum, data);
}


/*
 * CMD8 command (SEND_IF_COND)
 *
 * RETURN
 *     1 if valid response was received, 0 otherwise
 */
static int sdcard_cmd8_send()
{
    int retry;
    int ret;
    union sd_argument_t arg;
    union sd_response_t resp;

#define CHECK_PATTERN    0xAB
    arg.value = 0;
    arg.cmd08.check_pattern    = 0xAB;
    arg.cmd08.voltage_accepted = VOLTAGE_ACCEPTED_27_36;

    retry = 5;
    while (retry--)
    {
        ret = sdcard_hw_send_cmd(SD_CMD8_SEND_IF_COND, SD_CMDFLAG_EXPECT_SHORT, &arg, &resp);
        if (ret == 0)
        {
            /* 
             * NOTE
             * Check pattern not necessary. Card does not response if 
             * voltage not supported or sdcard is Ver1.X.
             */
            if (resp.R7.check_pattern == CHECK_PATTERN && resp.R7.voltage_accepted == VOLTAGE_ACCEPTED_27_36)
                return 1;
            else
                return 0;
        }
    }
    return 0;
}

/*
 * CMD55 (APP_CMD)
 *
 * RETURN
 *     1 if valid response was received, 0 otherwise
 */
static int sdcard_cmd55_send()
{
    int retry;
    int ret;
    union sd_argument_t arg;
    union sd_response_t resp;

    arg.value = 0;
    arg.cmd55.rca = cardstate.rca;

    retry = 5;
    while (retry--)
    {
        ret = sdcard_hw_send_cmd(SD_CMD55_APP_CMD, SD_CMDFLAG_EXPECT_SHORT, &arg, &resp);
        if (ret == 0)
        {
            /* TODO handle APP_CMD, ILLEGAL_COMMAND bits in response */
            if (resp.R1.app_cmd)
            {
                return 1;
            }
        }
    }
    return 0;
}

/*
 * ACMD41 (SD_SEND_OP_COND)
 *
 * RETURN
 *     type of detected card, SD_CARDTYPE_UNUSABLE on error
 */
static int sdcard_acmd41_send()
{
    int ctype;
    int retry;
    int ret;
    union sd_argument_t arg;
    union sd_response_t resp;

    arg.value = 0;
    /* XXX */
    arg.acmd41.v27_28 = 0;
    arg.acmd41.v28_29 = 0;
    arg.acmd41.v29_30 = 0;
    arg.acmd41.v30_31 = 0;
    arg.acmd41.v31_32 = 1;
    arg.acmd41.v32_33 = 1;
    arg.acmd41.v33_34 = 0;
    arg.acmd41.v34_35 = 0;
    arg.acmd41.v35_36 = 0;
    arg.acmd41.hcs    = 1;

    ctype = SD_CARDTYPE_UNUSABLE;

    retry = 1000;
    while (retry--)
    {
        if (sdcard_cmd55_send() == 0)
            continue;

         ret = sdcard_hw_send_cmd(SD_ACMD41_SD_SEND_OP_COND, SD_CMDFLAG_EXPECT_SHORT, &arg, &resp);
//        DPRINT("s4x s4x n", "ret ", ret, ", resp ", resp.sresp);
        /* NOTE ACMD41 always returned with CRC failed (CRC field set as 0b1111111) */
        if (ret & SD_STATUS_CMDTIMEOUT)
            continue;
        if (resp.R3.busy == 1)
        {
            if (resp.R3.ccs)
                ctype = SD_CARDTYPE_SD_V200_HC;
            else
                ctype = SD_CARDTYPE_SD_V200_SC;
            return ctype;
        }
    }
    return SD_CARDTYPE_UNUSABLE;
}

/*
 * CMD2 (ALL_SEND_CID)
 *
 * RETURN
 *     1 if valid response was received, 0 otherwise
 */
static int sdcard_cmd2_send()
{
    int retry;
    int ret;
    union sd_argument_t arg;
    union sd_response_t resp;

    char pnm[6];
    uint8 month;
    uint16 year;
    char oid[3];

    arg.value = 0;

    retry = 20;
    while (retry--)
    {
        ret = sdcard_hw_send_cmd(SD_CMD2_ALL_SEND_CID, SD_CMDFLAG_EXPECT_LONG, &arg, &resp);
//        DPRINT("s4x s4x n", "ret ", ret, ", resp ", resp.sresp);
        if (ret == 0)
        {
            oid[0] = resp.R2_CID.oid & 0xff;
            oid[1] = resp.R2_CID.oid >> 8;
            oid[2] = 0;

            pnm[0] = resp.R2_CID.pnm0;
            pnm[1] = resp.R2_CID.pnm1;
            pnm[2] = resp.R2_CID.pnm2;
            pnm[3] = resp.R2_CID.pnm3;
            pnm[4] = resp.R2_CID.pnm4;
            pnm[5] = 0;

            month = resp.R2_CID.mdt_month;
            year  = resp.R2_CID.mdt_year + 2000;

            DPRINT("tsn",          "--");
            DPRINT("ts1xn",        "Manuf. ID          : ", resp.R2_CID.mid);
            DPRINT("tssn",         "OEM/Application ID : ", oid);
            DPRINT("tssn",         "Prod. name         : ", pnm);
            DPRINT("ts 1d*.1d n",  "Prod. revision     : ", resp.R2_CID.prv >> 4, resp.R2_CID.prv & 0x0f);
            DPRINT("ts4xn",        "Prod. ser. number  : ", resp.R2_CID.psn);
            DPRINT("ts1d s2dn",    "Manuf. date        : ", month, ".", year);
            DPRINT("tsn",          "--");
            return 1;
        }
    }

    return 0;
}

/*
 * CMD3 (SEND_RELATIVE_ADDR)
 *
 * RETURN
 *     1 if valid response was received, 0 otherwise
 */
static int sdcard_cmd3_send()
{
    int retry;
    int ret;
    union sd_argument_t arg;
    union sd_response_t resp;

    arg.value = 0;

    retry = 20;
    while (retry--)
    {
        ret = sdcard_hw_send_cmd(SD_CMD3_SEND_RELATIVE_ADDR, SD_CMDFLAG_EXPECT_SHORT, &arg, &resp);
//        DPRINT("s4x s4x n", "ret ", ret, ", resp ", resp.sresp);
        if (ret == 0)
        {
            if (resp.R6.error)
                continue;

            cardstate.rca = resp.R6.rca;
            return 1;
        }
    }
    return 0;
}

/*
 * CMD7 (SELECT/DESELECT_CARD)
 *
 * RETURN
 *     1 if valid response was received, 0 otherwise
 */
static int sdcard_cmd7_send()
{
    int retry;
    int ret;
    union sd_argument_t arg;
    union sd_response_t resp;

    arg.value = 0;
    arg.ac.rca = cardstate.rca;

    retry = 20;
    while (retry--)
    {
        ret = sdcard_hw_send_cmd(SD_CMD7_SELECT_DESELECT_CARD, SD_CMDFLAG_EXPECT_SHORT, &arg, &resp);
        if (ret == 0)
        {
            return 1;
        }
    }
    return 0;
}



/*
 * CMD9 (SEND_CSD)
 *
 * RETURN
 *     1 if valid response was received, 0 otherwise
 */
static int sdcard_cmd9_send()
{
    int retry;
    int ret;
    union sd_argument_t arg;
    union sd_response_t resp;

    arg.value = 0;
    arg.ac.rca = cardstate.rca;

    retry = 0x200;
    while (retry--)
    {
        ret = sdcard_hw_send_cmd(SD_CMD9_SEND_CSD, SD_CMDFLAG_EXPECT_LONG, &arg, &resp);
//        DPRINT("s4x s4x n", "ret ", ret, ", resp ", resp.sresp);
        if (ret == 0)
        {
            /* 
             * XXX
             * At this time only CSD Version 2.0 is supported.
             * Card can return 3 as value of CSD_STRUCTURE bits, which is reserved value
             * in "Physical Layer Simplified Specification Version 4.10"
             *
             * "JESD84-B50 (eMMC5.0)"
             * Version is coded in the CSD_STRUCTURE byte in the EXT_CSD register
             *    0     CSD version No. 1.0
             *    1     CSD version No. 1.1 
             *    2     CSD version No. 1.2
             *    3     Version 4.1–4.2–4.3-4.41-4.5-4.51-5.0
             *          Version is coded in the CSD_STRUCTURE byte in the EXT_CSD register
             */
            if (resp.R2_CSD_v10.csd_structure != CSD_STRUCTURE_VERSION_20)
            {
                DPRINT("s1dn", "(W) Unknown/unimplemented CSD_STRUCTURE, ", resp.R2_CSD_v10.csd_structure);
                return 0;
            }
            memcpy(&cardstate.csd, &resp.R2_CSD_v10, sizeof (struct card_csd_v10_t));
            cardstate.blocklen = 1 << cardstate.csd.v20.read_bl_len;

            DPRINT("ts 4dn",  "Block size  : ", cardstate.blocklen);
            DPRINT("ts 4dsn", "Capacity    : ", (resp.R2_CSD_v20.c_size + 1) * 512 / 1024, " Mb");
            return 1;
        }
    }

    return 0;
}

/*
 * CMD12 (STOP_TRANSMISSION)
 *
 * RETURN
 *     1 if valid response was received, 0 otherwise
 */
static int sdcard_cmd12_send()
{
    int retry;
    int ret;
    union sd_argument_t arg;
    union sd_response_t resp;

    arg.value = 0;
    arg.ac.rca = cardstate.rca;

    retry = 20;
    while (retry--)
    {
        ret = sdcard_hw_send_cmd(SD_CMD12_STOP_TRANSMISSION, SD_CMDFLAG_EXPECT_SHORT, &arg, &resp);
        if (ret == 0)
        {
            /* XXX response is R1b, is busy checked by sd card controller? */
            return 1;
        }
    }
    return 0;
}


/*
 * ACMD6 (SET_BUS_WIDTH)
 *
 * RETURN
 *     1 if valid response was received, 0 otherwise
 */
static int sdcard_acmd6_send()
{
    int retry;
    int ret;
    union sd_argument_t arg;
    union sd_response_t resp;

    arg.value = 0;
    arg.acmd6.bus_width = ACMD6_BUS_WIDTH_4;

    retry = 20;
    while (retry--)
    {
        if (sdcard_cmd55_send() == 0)
            continue;

        ret = sdcard_hw_send_cmd(SD_ACMD6_SET_BUS_WISTH, SD_CMDFLAG_EXPECT_SHORT, &arg, &resp);
//        DPRINT("s4x s4x n", "ret ", ret, ", resp ", resp.sresp);
        if (ret == 0)
        {
            return 1;
        }
    }
    return 0;
}

/*
 * NOTE following code is based on code from u-boot-1.1.6, patches from EA for
 * LPC2468 OEM Board
 */

/******************************************************************************
 ** Function name:		MCI_CheckStatus
 **
 ** Descriptions:		MCI Check status before and after the block read and 
 **						write. Right after the block read and write, this routine 
 **						is important that, even the FIFO is empty, complete
 **						block has been sent, but, data is still being written
 **						to the card, this routine is to ensure that the data
 **						has been written based on the state of the card, not
 **						by the length being set.  
 **
 ** parameters:			None
 ** Returned value:             1 on success, 0 otherwise
 ** 
 ******************************************************************************/
static int sdcard_checkstatus()
{
    int retry;
    int ret;
    union sd_argument_t arg;
    union sd_response_t resp;

    arg.value = 0;
    arg.ac.rca = cardstate.rca;

    retry = 1000;
    while (retry--)
    {
        ret = sdcard_hw_send_cmd(SD_CMD13_SEND_STATUS, SD_CMDFLAG_EXPECT_SHORT, &arg, &resp);
        if (ret == 0)
            break;
    }

    /*
     * The only valid state is TRANS per MMC and SD state diagram.
     * RCV state may be seen, but, I have found that it happens
     * only when TX_ACTIVE or RX_ACTIVE occurs before the WRITE_BLOCK and 
     * READ_BLOCK cmds are being sent, which is not a valid sequence.
     */  
    if (resp.R1.ready_for_data && resp.R1.current_state == R1_CURRENT_STATE_TRAN)
        return 1;

    return 0;
}

/******************************************************************************
 ** Function name:		MCI_Send_Stop
 **
 ** Descriptions:		CMD12, STOP_TRANSMISSION. if that happens, the card is 
 **						maybe in a unknown state that need a warm reset.		
 **
 ** parameters:			None
 ** Returned value:             1 on success, 0 otherwise
 ** 
 ******************************************************************************/
static int sdcard_stop_transmission()
{
    int retry;
    int ret;
    union sd_argument_t arg;
    union sd_response_t resp;

    arg.value = 0;
    arg.ac.rca = cardstate.rca;

    retry = 0x20;
    while (retry--)
    {
        ret = sdcard_hw_send_cmd(SD_CMD12_STOP_TRANSMISSION, SD_CMDFLAG_EXPECT_SHORT, &arg, &resp);
        if (ret == 0)
        {
            /* ready bit, bit 8, should be set in the card status register */
            if (resp.R1.ready_for_data)
                return 1;
        }
    }

    return 0;
}

/******************************************************************************
 ** Function name:		MCI_Send_Read_Block
 **
 ** Descriptions:		CMD17, READ_SINGLE_BLOCK, send this cmd in the TRANS 
 **						state to read a block of data from the card.
 **
 ** parameters:			block number
 ** Returned value:		Response value
 ** 
 ******************************************************************************/
static int sdcard_cmd_read_block(uint32 baddr)
{
    union sd_argument_t arg;
    union sd_response_t resp;
    int retry;

    arg.value = baddr;

    retry = 0x20;
    while (retry--)
    {
        if (sdcard_hw_send_cmd(
                    SD_CMD17_READ_SINGLE_BLOCK, SD_CMDFLAG_EXPECT_SHORT,
                    &arg, &resp) == 0)
        {
            /* it should be in the transfer state, bit 9~12 is 0x0100 and bit 8 is 1 */
            if (resp.R1.ready_for_data && resp.R1.current_state == R1_CURRENT_STATE_TRAN)
                return 1;
        }
    }


    return 0;
}
/******************************************************************************
 ** Function name:		MCI_Read_Block
 **
 ** Descriptions:		Set MCI data control register, data length and data
 **						timeout, send READ_SINGLE_BLOCK cmd, finally, enable
 **						interrupt. On completion of READ_SINGLE_BLOCK cmd, 
 **						RX_ACTIVE interrupt will occurs, data can be read 
 **						continuously into the FIFO until the block data 
 **						length is reached.		
 **
 ** parameters:			block number
 ** Returned value:		true or false, if cmd times out, return false and no 
 **						need to continue.
 **
 ** 
 ******************************************************************************/
int sdcard_read_block(uint32 bnum, uint8 *data)
{
    int retry;

    retry = 5;
    while (retry--)
    {
        /* Below status check is redundant, but ensure card is in TRANS state
           before writing and reading to from the card. */
        if (!sdcard_checkstatus())
        {
            DPRINT("sn", "(E) sdcard_checkstatus failed");
            sdcard_stop_transmission();
            return 0;
        }

        /*
         * NOTE
         * for SDHC/SDXC cards parameter is block number, for SDSC
         * parameter is byte unit address
         *
         * XXX only SDHC/SDXC implemented
         */
        if (!sdcard_cmd_read_block(bnum))
        {
            DPRINT("sn", "(E) sdcard_cmd_read_block failed");
            return 0;
        }

        if (sdcard_hw_read_block(data))
        {
            return 1;
        } else {
            /* XXX what is shown by block number? */
            DEBUG_EMSGF("hwread", "<block >4xn", bnum);

            /* send CMD12 */
            if (!sdcard_cmd12_send())
            {
                DEBUG_EMSG("CMD12 FAILED");
                return 0;
            }
        }
    }

    return 0;
}


/******************************************************
 * high level functions for fat_io_lib
 ******************************************************
 */
//static uint8 block[512] __attribute__((section("sram")));


int media_read(uint32 sector, uint8 *buffer, uint32 sector_count)
{
    while (sector_count--)
    {
        if (!sdcard_read(sector++, buffer))
        {
            dprint("sn", "(E) sdcard read error");
            return 0;
        }
        buffer += 512;
    }

    return 1;
}

int media_write(uint32 sector, uint8 *buffer, uint32 sector_count)
{
    unsigned long i;

    dprint("sn", "media write not implemented");
    return 0;

    for (i=0;i<sector_count;i++)
    {
        // ...
        // Add platform specific sector (512 bytes) write code here
        //..

        sector ++;
        buffer += 512;
    }

    return 1;
}

