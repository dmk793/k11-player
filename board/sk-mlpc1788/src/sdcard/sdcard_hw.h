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

#ifndef SDCARD_HW_H
#define SDCARD_HW_H

#include <types.h>


/************************************************
 * 
 ***********************************************/
/*
 * Power control register - PWR
 */
#define SD_PWR_CTRL_POWER_OFF    (0x00 << 0)
#define SD_PWR_CTRL_POWER_UP     (0x02 << 0)
#define SD_PWR_CTRL_POWER_ON     (0x03 << 0)
#define SD_PWR_OPENDRAIN         (1 << 6)
#define SD_PWR_ROD               (1 << 7)
/*
 * Clock control register - CLOCK
 *
 * Note: After a data write, data cannot be written to this register for three MCLK clock
 * periods plus two PCLK clock periods.
 */
#define SD_CLOCK_CLKDIV_MASK    0xff
#define SD_CLOCK_CLKDIV(div)    ((div) << 0)
#define SD_CLOCK_ENABLE         (1 << 8)  /* enable SD card bus clock */
#define SD_CLOCK_PWRSAVE        (1 << 9)  /* disable SD_CLK output when bus is idle,
                                           *     0 - always enabled,
                                           *     1 - clock enabled when bus is active
                                           */
#define SD_CLOCK_BYPASS         (1 << 10) /* enable bypass clock of divide logic, 1 - enable */
#define SD_CLOCK_WIDEBUS        (1 << 11) /* enable wide bus mode, 1 - 4 -bit data bus */
/*
 * Command register - COMMAND
 */
#define SD_COMMAND_CMDINDEX_MASK    0x3f
#define SD_COMMAND_CMDINDEX(cmd)    ((cmd) << 0)
#define SD_COMMAND_RESPONSE         (1 << 6)     /* CPSM waits for response */
#define SD_COMMAND_LONG             (1 << 7)     /* CPSM receives long response */
#define SD_COMMAND_INTERRUPT        (1 << 8)     /* CPSM waits for interrupt request */ 
#define SD_COMMAND_PENDING          (1 << 9)     /* CPSM waits for CmdPend before it starts sending command */
#define SD_COMMAND_ENABLE           (1 << 10)    /* CPSM is enabled */
/*
 * MCI status register - STATUS
 */
/* this bits can be cleared by CLEAR register */
#define SD_STATUS_CMDCRCFAIL       (1 << 0)  /* command response received (CRC check failed) */
#define SD_STATUS_DATACRCFAIL      (1 << 1)  /* data block sent/received (CRC check failed) */
#define SD_STATUS_CMDTIMEOUT       (1 << 2)  /* command response timeout */
#define SD_STATUS_DATATIMEOUT      (1 << 3)  /* data timeout */
#define SD_STATUS_TXUNDERRUN       (1 << 4)  /* transmit fifo underrun error */
#define SD_STATUS_RXOVERRUN        (1 << 5)  /* receive fifo overrun error */
#define SD_STATUS_CMDRESPEND       (1 << 6)  /* command response received (CRC check passed) */
#define SD_STATUS_CMDSENT          (1 << 7)  /* command sent (no response required) */
#define SD_STATUS_DATAEND          (1 << 8)  /* data end (data counter is zero) */
#define SD_STATUS_STARTBITERR      (1 << 9)  /* start bit not detected on all data signals in wide bus mode */
#define SD_STATUS_DATABLOCKEND     (1 << 10) /* data block sent/received (CRC check passed) */
/* this bits updated dynamicly */
#define SD_STATUS_CMDACTIVE        (1 << 11) /* command transfer in progress */
#define SD_STATUS_TXACTIVE         (1 << 12) /* data transmit in progress */
#define SD_STATUS_RXACTIVE         (1 << 13) /* data receive in progress */
#define SD_STATUS_TXFIFOHALFEMPTY  (1 << 14) /* transmit FIFO half empty */
#define SD_STATUS_RXFIFOHALFFULL   (1 << 15) /* receive FIFO half full */
#define SD_STATUS_TXFIFOFULL       (1 << 16) /* transmit FIFO full */
#define SD_STATUS_RXFIFOFULL       (1 << 17) /* receive FIFO full */
#define SD_STATUS_TXFIFOEMPTY      (1 << 18) /* transmit FIFO empty */
#define SD_STATUS_RXFIFOEMPTY      (1 << 19) /* receive FIFO empty */
#define SD_STATUS_TXDATAAVLBL      (1 << 20) /* data available in transmit FIFO */
#define SD_STATUS_RXDATAAVLBL      (1 << 21) /* data available in receive FIFO */

#define SD_CLEAR_MASK    0x7ff
/*
 * MCI Data Control Register - DATACTRL
 */
#define SD_DATACTRL_ENABLE           (1 << 0)    /* data transfer enable */
#define SD_DATACTRL_DIRECTION_WRITE  (0 << 1)    /* from controller to card */
#define SD_DATACTRL_DIRECTION_READ   (1 << 1)    /* from card to controller */
#define SD_DATACTRL_MODE_BLOCK       (0 << 2)    /* block data tranfser */
#define SD_DATACTRL_MODE_STREAM      (1 << 2)    /* stream data tranfser */
#define SD_DATACTRL_DMAENABLE        (1 << 3)    /* DMA enabled */
#define SD_DATACTRL_BLOCKSIZE_MASK   (0xf << 4)  /* data block length */
#define SD_DATACTRL_BLOCKSIZE(bs)    ((bs) << 4) /* data block length */



/************************************************
 * Physical Layer Simplified Specification v4.10. Tables 4-22 to 4-32.
 ***********************************************/
#define SD_CMD0_GO_IDLE_STATE        0    /* bc   response -   */
#define SD_CMD2_ALL_SEND_CID         2    /* bcr  response R2  */
#define SD_CMD3_SEND_RELATIVE_ADDR   3    /* bcr  response R6  */
#define SD_CMD7_SELECT_DESELECT_CARD 7    /* ac   response R1b */
#define SD_CMD8_SEND_IF_COND         8    /* bcr  response R7  */
#define SD_CMD9_SEND_CSD             9    /* ac   response R2  */
#define SD_CMD12_STOP_TRANSMISSION   12   /* ac   response R1b */
#define SD_CMD13_SEND_STATUS         13   /* ac   response R1  */
#define SD_CMD17_READ_SINGLE_BLOCK   17   /* ac   response R1  */
#define SD_CMD55_APP_CMD             55   /* ac   response R1  */
#define SD_ACMD6_SET_BUS_WISTH       6    /* ac   response R1  */
#define SD_ACMD41_SD_SEND_OP_COND    41   /* bcr  response R3  */

#pragma pack(push, 1)
union sd_argument_t {
    struct {
        uint32 check_pattern    : 8;
#define VOLTAGE_ACCEPTED_27_36           0x01
#define VOLTAGE_ACCEPTED_RESERVED_LOW    0x02
        uint32 voltage_accepted : 4;
        uint32 reserved         : 20;
    } cmd08;

    struct {
        uint32 reserved   : 16;
        uint32 rca        : 16;
    } ac;

    /* OCR */
    struct {
        uint32 reserved00 : 15; /* 0 - 14 */
        uint32 v27_28     : 1;  /* 15 */
        uint32 v28_29     : 1;  /* 16 */
        uint32 v29_30     : 1;  /* 17 */
        uint32 v30_31     : 1;  /* 18 */
        uint32 v31_32     : 1;  /* 19 */
        uint32 v32_33     : 1;  /* 20 */
        uint32 v33_34     : 1;  /* 21 */
        uint32 v34_35     : 1;  /* 22 */
        uint32 v35_36     : 1;  /* 23 */
        uint32 s18r       : 1;  /* 24 */
        uint32 reserved01 : 3;  /* 25 - 27 */
        uint32 xpc        : 1;  /* 28   SDXC power control */ 
        uint32 fb         : 1;  /* 29 */
        uint32 hcs        : 1;  /* 30   Host Capacity Support, 0 - SDSC only, 1 - SDHC or SDXC */
        uint32 busy       : 1;  /* 31 */
    } acmd41;

    struct {
#define ACMD6_BUS_WIDTH_1    0
#define ACMD6_BUS_WIDTH_4    2
        uint32 bus_width  : 2; /* 0 - 1 */
        uint32 reserved   : 30;
    } acmd6;

    struct {
        uint32 reserved   : 16;
        uint32 rca        : 16;
    } cmd55;

    uint32 value;
};

union sd_response_t {
    /* short responses, bits 39:8 */ 
    uint32 sresp;
    /* long response, bits 127:1 */
    uint32 lresp[4];

    /* card status */
    struct {
        uint32 reserved00         : 3; /* 0 - 2   */
        uint32 ake_seq_error      : 1; /* 3       Error in sequence of the authentication process,
                                        *             0 - no error */
        uint32 reserved01         : 1; /* 4       */
        uint32 app_cmd            : 1; /* 5       Card will expect ACMD, 
                                                      1 - enabled, 0 - disabled */
        uint32 reserved02         : 2; /* 6 - 7   */
        uint32 ready_for_data     : 1; /* 8       Corresponds to buffer empty signaling on the bus */
#define R1_CURRENT_STATE_IDLE    0
#define R1_CURRENT_STATE_READY   1
#define R1_CURRENT_STATE_IDENT   2
#define R1_CURRENT_STATE_STBY    3
#define R1_CURRENT_STATE_TRAN    4
#define R1_CURRENT_STATE_DATA    5
#define R1_CURRENT_STATE_RCV     6
#define R1_CURRENT_STATE_PRG     7
#define R1_CURRENT_STATE_DIS     8
        uint32 current_state      : 4; /* 9 - 12  */
        uint32 erase_state        : 1; /* 13      Erase sequence was cleared before executing. 
                                        *             0 - cleared, 1 - set */
        uint32 card_ecc_disabled  : 1; /* 14      0 - enabled, 1 - disabled */
        uint32 wp_erase_skip      : 1; /* 15      Set when only partial address space was erase due
                                        *         to existing write protection. */
        uint32 csd_overwrite      : 1; /* 16      
                                        *             0 - no error, 1 - error */
        uint32 reserved03         : 2; /* 17 - 18 */
        uint32 error              : 1; /* 19      A general or an unknown error occured */
        uint32 cc_error           : 1; /* 20      Internal card controller error
                                        *             0 - no error, 1 - error */
        uint32 card_ecc_failed    : 1; /* 21      Card internal ECC was applied but failed to
                                        *         correct the data */
        uint32 illegal_command    : 1; /* 22      Command not legal for the card state
                                        *             0 - no error, 1 - error */
        uint32 com_crc_error      : 1; /* 23      The CRC check of previous command failed
                                        *             0 - no error, 1 - error */
        uint32 lock_unlock_failed : 1; /* 24      Set when a sequence or passward error has been
                                        *         detected in lock/unlock card command
                                        *             0 - no error, 1 - error */
        uint32 card_is_locked     : 1; /* 25      When set, signals that the card is locked by
                                                  the host
                                                      0 - card unlocked, 1 card locked */
        uint32 wp_violation       : 1; /* 26      Set when the host attempts to write to
                                        *         a protected block or to the temporary or
                                        *         permanent write protected card
                                        *             0 - not protected, 1 - protected
                                        */
        uint32 erase_param        : 1; /* 27      An invalid selection of write-blocks for erase
                                        *         occured
                                        *             0 - no error, 1 - error */
        uint32 erase_seq_error    : 1; /* 28      An error in the sequence of erase commands
                                        *         occured
                                        *             0 - no error, 1 - error */
        uint32 block_len_error    : 1; /* 29      The transferred block length is not allowed
                                        *         for this card, or the number of transferred
                                        *         bytes does not match the block length
                                        *             0 - no error, 1 - error */
        uint32 address_error      : 1; /* 30      A misaligned address which did not match the
                                        *         block length was used in the command
                                        *             0 - no error, 1 - error */
        uint32 out_of_range       : 1; /* 31      The command's argument was out of the allowed
                                        *         range for this card.
                                        *             0 - no error, 1 - error */
    } R1;

    /* OCR */
    struct {
        uint32 reserved00 : 15; /* 0 - 14 */
        uint32 v27_28     : 1;  /* 15 */
        uint32 v28_29     : 1;  /* 16 */
        uint32 v29_30     : 1;  /* 17 */
        uint32 v30_31     : 1;  /* 18 */
        uint32 v31_32     : 1;  /* 19 */
        uint32 v32_33     : 1;  /* 20 */
        uint32 v33_34     : 1;  /* 21 */
        uint32 v34_35     : 1;  /* 22 */
        uint32 v35_36     : 1;  /* 23 */
        uint32 s18a       : 1;  /* 24 */
        uint32 reserved01 : 4;  /* 25 - 28 */
        uint32 uhs2       : 1;  /* 29   0 - Non UHS-II card, 1 - UHS-II card */
        uint32 ccs        : 1;  /* 30   Card Capacity Status, 0 - SDSC, 1 - SDHC or SDXC */
        uint32 busy       : 1;  /* 31 */
    } R3;

    /* CSD, CID */
    struct {
        uint32 reserved00 : 1;  /*  0 */
        uint32 crc7       : 7;  /*  1 - 7 */
        uint32 mdt_month  : 4;  /*  8 - 11    Manufacturing month, 1 - Jan */
        uint32 mdt_year   : 8;  /* 12 - 19    Manufacturing year, 0 - 2000 */
        uint32 reserved01 : 4;  /* 20 - 23 */
        uint32 psn        : 32; /* 24 - 55    Product serial number */
        uint32 prv        : 8;  /* 56 - 63    Product revision */
        uint32 pnm0       : 8;  /* 64 - 103   Product name */
        uint32 pnm1       : 8;  /*                         */
        uint32 pnm2       : 8;  /*                         */
        uint32 pnm3       : 8;  /*                         */
        uint32 pnm4       : 8;  /*                         */
        uint32 oid        : 16; /* 104 - 119  OEM/Application ID */
        uint32 mid        : 8;  /* 120 - 127  Manufacturer ID */
    } R2_CID;
    struct card_csd_v10_t {
        /* TODO */
        uint32 reserved00    : 32;
        uint32 reserved01    : 32;
        uint32 reserved02    : 32;
        uint32 reserved03    : 30;
#define CSD_STRUCTURE_VERSION_10   0
#define CSD_STRUCTURE_VERSION_20   1
        uint32 csd_structure : 2;  /* 126 - 127 CSD structure version */
    } R2_CSD_v10;
    struct card_csd_v20_t {
        uint32 reserved00         : 1;  /* 0         */
        uint32 crc                : 7;  /* 1 - 7     */
        uint32 reserved01         : 2;  /* 8 - 9     */
        uint32 file_format        : 2;  /* 10 - 11   */
        uint32 tmp_write_protect  : 1;  /* 12        */
        uint32 perm_write_protect : 1;  /* 13        */
        uint32 copy               : 1;  /* 14        */
        uint32 file_format_grp    : 1;  /* 15        */
        uint32 reserved02         : 5;  /* 16 - 20   */
        uint32 write_bl_partial   : 1;  /* 21        */
        uint32 write_bl_len       : 4;  /* 22 - 25   */
        uint32 r2w_factor         : 3;  /* 26 - 28   */
        uint32 reserved03         : 2;  /* 29 - 30   */
        uint32 wr_grp_enable      : 1;  /* 31        */
        uint32 wr_grp_size        : 7;  /* 32 - 38   */
        uint32 sector_size        : 7;  /* 39 - 45   */
        uint32 erase_blk_en       : 1;  /* 46        */
        uint32 reserved04         : 1;  /* 47        */
        uint32 c_size             : 22; /* 48 - 69   */
        uint32 reserved05         : 6;  /* 70 - 75   */
        uint32 dsr_imp            : 1;  /* 76        */
        uint32 read_blk_misalign  : 1;  /* 77        */
        uint32 write_blk_misalign : 1;  /* 78        */
        uint32 read_bl_partial    : 1;  /* 79        */
        uint32 read_bl_len        : 4;  /* 80 - 83   */
        uint32 ccc                : 12; /* 84 - 95   */
        uint32 tran_speed         : 8;  /* 96 - 103  */
        uint32 nsac               : 8;  /* 104 - 111 */
        uint32 taac               : 8;  /* 112 - 119 */
        uint32 reserved06         : 6;  /* 120 - 125 */
        uint32 csd_structure      : 2;  /* 126 - 127 CSD structure version */
    } R2_CSD_v20;

    struct {
        /* 23, 22, 19, 12:0 bits of card status */
        uint32 reserved00         : 3; /* 0 - 2   */
        uint32 ake_seq_error      : 1; /* 3       Error in sequence of the authentication process,
                                        *             0 - no error */
        uint32 reserved01         : 1; /* 4       */
        uint32 app_cmd            : 1; /* 5       Card will expect ACMD, 
                                                      1 - enabled, 0 - disabled */
        uint32 reserved02         : 2; /* 6 - 7   */
        uint32 ready_for_data     : 1; /* 8       Corresponds to buffer empty signaling on the bus */
        uint32 current_state      : 4; /* 9 - 12  */
        uint32 error              : 1; /* 19      A general or an unknown error occured */
        uint32 illegal_command    : 1; /* 22      Command not legal for the card state
                                        *             0 - no error, 1 - error */
        uint32 com_crc_error      : 1; /* 23      The CRC check of previous command failed
                                        *             0 - no error, 1 - error */
        uint32 rca                : 16;
    } R6;

    struct {
        uint32 check_pattern    : 8;
        uint32 voltage_accepted : 4;
        uint32 reserved         : 20;
    } R7;

};
#pragma pack(pop)

/************************************************
 * 
 ***********************************************/
void sdcard_hw_init();
void sdcard_hw_set_lo_clk();
void sdcard_hw_set_hi_clk();
int sdcard_hw_read_block(uint8 *data);
int card_detect();

#define SD_CMDFLAG_NO_RESPONSE      0
#define SD_CMDFLAG_EXPECT_SHORT     SD_COMMAND_RESPONSE
#define SD_CMDFLAG_EXPECT_LONG     (SD_COMMAND_RESPONSE | SD_COMMAND_LONG)
int sdcard_hw_send_cmd(uint8 cmd, uint32 flags, union sd_argument_t *arg, union sd_response_t *resp);


#endif

