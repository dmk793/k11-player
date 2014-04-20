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
 * miscellaneous definitions releted to ethernet
 */
#ifndef ETH_DEF_H
#define ETH_DEF_H

#define EMAC_MAC1_RECEIVE_ENABLE          (1 << 0)
#define EMAC_MAC1_PASS_ALL_RECEIVE_FRAMES (1 << 1)
#define EMAC_MAC1_RX_FLOW_CONTROL         (1 << 2)
#define EMAC_MAC1_TX_FLOW_CONTROL         (1 << 3)
#define EMAC_MAC1_LOOPBACK                (1 << 4)
#define EMAC_MAC1_RESET_TX                (1 << 8)
#define EMAC_MAC1_RESET_MCS_TX            (1 << 9)
#define EMAC_MAC1_RESET_RX                (1 << 10)
#define EMAC_MAC1_RESET_MCS_RX            (1 << 11)
#define EMAC_MAC1_SIMULATION_RESET        (1 << 14)
#define EMAC_MAC1_SOFT_RESET              (1 << 15)

#define EMAC_COMMAND_RX_ENABLE            (1 << 0)
#define EMAC_COMMAND_TX_ENABLE            (1 << 1)
#define EMAC_COMMAND_REG_RESET            (1 << 3)
#define EMAC_COMMAND_TX_RESET             (1 << 4)
#define EMAC_COMMAND_RX_RESET             (1 << 5)
#define EMAC_COMMAND_PASS_RUNT_FRAME      (1 << 6)
#define EMAC_COMMAND_PASS_RX_FILTER       (1 << 7)
#define EMAC_COMMAND_TX_FLOW_CONTROL      (1 << 8)
#define EMAC_COMMAND_RMII                 (1 << 9)
#define EMAC_COMMAND_FULL_DUPLEX          (1 << 10)

/* MII Management Configuration Register */
#define EMAC_MCFG_SCAN_INC       (1 << 0)     /* Scan Increment PHY Address     */
#define EMAC_MCFG_SUPP_PREAM     (1 << 1)     /* Suppress Preamble              */
#define EMAC_MCFG_RES_MII        (1 << 15)    /* Reset MII Management Hardware  */

#define EMAC_MCFG_CLK_DIV4       (0x00 << 2)  /* MDC = hclk / 4   */
#define EMAC_MCFG_CLK_DIV6       (0x02 << 2)  /* MDC = hclk / 6   */
#define EMAC_MCFG_CLK_DIV8       (0x03 << 2)  /* MDC = hclk / 8   */
#define EMAC_MCFG_CLK_DIV10      (0x04 << 2)  /* MDC = hclk / 10  */
#define EMAC_MCFG_CLK_DIV14      (0x05 << 2)  /* MDC = hclk / 14  */
#define EMAC_MCFG_CLK_DIV20      (0x06 << 2)  /* MDC = hclk / 20  */
#define EMAC_MCFG_CLK_DIV28      (0x07 << 2)  /* MDC = hclk / 28  */

/* MII Management Command Register */
#define EMAC_MCMD_READ           (1 << 0)  /* MII Read                          */
#define EMAC_MCMD_SCAN           (1 << 1)  /* MII Scan continuously             */

/* PHY Support Register */
#define EMAC_SUPP_SPEED          (1 << 8)   /* Reduced MII Logic Current Speed   */
#define EMAC_SUPP_RES_RMII       (1 << 11)  /* XXX undocumented or error ? XXX Reset Reduced MII Logic           */

/* MII Management Indicators Register */
#define EMAC_MIND_BUSY           (1 << 0)  /* MII is Busy                       */
#define EMAC_MIND_SCAN           (1 << 1)  /* MII Scanning in Progress          */
#define EMAC_MIND_NOT_VAL        (1 << 2)  /* MII Read Data not valid           */
#define EMAC_MIND_MII_LINK_FAIL  (1 << 3)  /* MII Link Failed                   */

/* =================================================
 * PHY Register
 * =================================================
 */
/* KS8721BL registers */
#define PHY_KS8721BL_BMCR             0x0000
#define PHY_KS8721BL_BMSR             0x0001
#define PHY_KS8721BL_PHYIDR1          0x0002
#define PHY_KS8721BL_PHYIDR2          0x0003
#define PHY_KS8721BL_ANAR             0x0004
#define PHY_KS8721BL_ANLPAR           0x0005
#define PHY_KS8721BL_ANER             0x0006
#define PHY_KS8721BL_ANNPTR           0x0007
#define PHY_KS8721BL_LPNPA            0x0008
#define PHY_KS8721BL_RXER_CNT         0x0015
#define PHY_KS8721BL_INT_CTRL         0x001B
#define PHY_KS8721BL_LINKMD_CTRL      0x001D
#define PHY_KS8721BL_PHY_CTRL         0x001E
#define PHY_KS8721BL_100BASE_PHYCTRL  0x001F

/* BMCR setting */
#define PHY_KS8721BL_BMCR_RESET          0x8000
#define PHY_KS8721BL_BMCR_LOOPBACK       0x4000
#define PHY_KS8721BL_BMCR_SPEED_100      0x2000
#define PHY_KS8721BL_BMCR_AN             0x1000
#define PHY_KS8721BL_BMCR_POWERDOWN      0x0800
#define PHY_KS8721BL_BMCR_ISOLATE        0x0400
#define PHY_KS8721BL_BMCR_RE_AN          0x0200
#define PHY_KS8721BL_BMCR_DUPLEX         0x0100
#define PHY_KS8721BL_BMCR_COLS_TEST      0x0080
#define PHY_KS8721BL_BMCR_DIS_TRANSM     0x0001

/* BMSR setting */
#define PHY_KS8721BL_BMSR_100BE_T4           0x8000
#define PHY_KS8721BL_BMSR_100TX_FULL         0x4000
#define PHY_KS8721BL_BMSR_100TX_HALF         0x2000
#define PHY_KS8721BL_BMSR_10BE_FULL          0x1000
#define PHY_KS8721BL_BMSR_10BE_HALF          0x0800
#define PHY_KS8721BL_BMSR_AUTO_DONE          0x0020
#define PHY_KS8721BL_BMSR_REMOTE_FAULT       0x0010
#define PHY_KS8721BL_BMSR_NO_AUTO            0x0008
#define PHY_KS8721BL_BMSR_LINK_ESTABLISHED   0x0004

/* PHY 100BASE-TX PHYCTRL setting */
#define PHY_KS8721BL_100BASE_PHYCTRL_DISDS              (1 << 0)    /* disable data scramblin */
#define PHY_KS8721BL_100BASE_PHYCTRL_ESQET              (1 << 1)    /* enable SQE test */
#define PHY_KS8721BL_100BASE_PHYCTRL_OPMODE_MASK        (0x07 << 2) /* [000] = Still in auto-negotiation.  */
#define PHY_KS8721BL_100BASE_PHYCTRL_OPMODE_AN          (0x00 << 2) /* [000] = Still in auto-negotiation.  */
#define PHY_KS8721BL_100BASE_PHYCTRL_OPMODE_10BT_HD     (0x01 << 2) /* [001] = 10BASE-T half-duplex.  */
#define PHY_KS8721BL_100BASE_PHYCTRL_OPMODE_100BT_HD    (0x02 << 2) /* [010] = 100BASE-TX half-duplex.  */
#define PHY_KS8721BL_100BASE_PHYCTRL_OPMODE_10BT_FD     (0x05 << 2) /* [101] = 10BASE-T full-duplex.  */
#define PHY_KS8721BL_100BASE_PHYCTRL_OPMODE_100BT_FD    (0x06 << 2) /* [110] = 100BASE-TX full-duplex.  */
#define PHY_KS8721BL_100BASE_PHYCTRL_OPMODE_PHY_ISOLATE (0x07 << 2) /* [111] = PHY/MII isolate.  */
#define PHY_KS8721BL_100BASE_PHYCTRL_PHY_ISOLATE        (1 << 5)    /* PHY isolate */
#define PHY_KS8721BL_100BASE_PHYCTRL_EN_PAUSE           (1 << 6)    /* enable pause */
#define PHY_KS8721BL_100BASE_PHYCTRL_AN_COMPLETE        (1 << 7)    /* auto-negotiation complete */
#define PHY_KS8721BL_100BASE_PHYCTRL_EN_JABBER          (1 << 8)    /* enable jabber */
#define PHY_KS8721BL_100BASE_PHYCTRL_INT_LEVEL          (1 << 9)    /* interrupt level */
#define PHY_KS8721BL_100BASE_PHYCTRL_PWR_SAVE           (1 << 10)   /* enable power saving */
#define PHY_KS8721BL_100BASE_PHYCTRL_FORCE_LINK         (1 << 11)   /* force link pass */
#define PHY_KS8721BL_100BASE_PHYCTRL_ENRG_DETECT        (1 << 12)   /* presense signal on RX+/RX- analog wite pair */
#define PHY_KS8721BL_100BASE_PHYCTRL_PWR_SWAP           (1 << 13)   /* disable MDI/MDI-X */


#define SPEED_100           1
#define SPEED_10            0
#define FULL_DUPLEX         1
#define HALF_DUPLEX         0

extern unsigned int *_ethram;      /* ethernet ram address, defined in linker script */
extern unsigned int *_ethram_size; /* ethernet ram size, defined in linker script */

#define EMAC_RAM_ADDR       ((uint32)&_ethram)
#define EMAC_RAM_SIZE       ((uint32)&_ethram_size)

#define EMAC_TX_DESCRIPTOR_COUNT    5
#define EMAC_RX_DESCRIPTOR_COUNT    5

/*
 * (Rx|Tx)Descriptor ::
 *   [4] packet  - base address of the buffer containing the data
 *   [4] control - control information
 */
#define TX_DESCRIPTOR_SIZE  (EMAC_TX_DESCRIPTOR_COUNT * 8)
#define RX_DESCRIPTOR_SIZE  (EMAC_RX_DESCRIPTOR_COUNT * 8)

/* 
 * TxStatus ::
 *   [4] status
 */
#define TX_STATUS_SIZE      (EMAC_TX_DESCRIPTOR_COUNT * 4)

/* 
 * RxStatus ::
 *   [4] status        - receive status flags
 *   [4] statusHashCRC - concat of dest addr hash CRC and src addr CRC
 */
#define RX_STATUS_SIZE      (EMAC_RX_DESCRIPTOR_COUNT * 8)


#define TOTAL_DESCRIPTOR_SIZE   (TX_DESCRIPTOR_SIZE + RX_DESCRIPTOR_SIZE + TX_STATUS_SIZE + RX_STATUS_SIZE)

/* descriptors are placed at the end of the emac address space */
#define EMAC_DESCRIPTOR_ADDR    (EMAC_RAM_ADDR + EMAC_RAM_SIZE - TOTAL_DESCRIPTOR_SIZE) 

#define TX_DESCRIPTOR_ADDR  EMAC_DESCRIPTOR_ADDR
#define TX_STATUS_ADDR      (EMAC_DESCRIPTOR_ADDR + TX_DESCRIPTOR_SIZE)
#define RX_DESCRIPTOR_ADDR  (TX_STATUS_ADDR + TX_STATUS_SIZE)
#define RX_STATUS_ADDR      (RX_DESCRIPTOR_ADDR + RX_DESCRIPTOR_SIZE)

//#define EMAC_DMA_ADDR       EMAC_RAM_ADDR
//#define EMAC_DMA_SIZE       EMAC_RAM_ADDR + EMAC_RAM_END - TOTAL_DESCRIPTOR_SIZE


#define EMAC_BLOCK_SIZE         0x600
#define EMAC_TX_BLOCK_NUM       5   
#define EMAC_RX_BLOCK_NUM       5
//#define TOTAL_EMAC_BLOCK_NUM    10

//#define EMAC_BUFFER_SIZE    (EMAC_BLOCK_SIZE * (EMAC_TX_BLOCK_NUM + EMAC_RX_BLOCK_NUM ))
#define EMAC_TX_BUFFER_ADDR EMAC_RAM_ADDR
#define EMAC_RX_BUFFER_ADDR (EMAC_RAM_ADDR + EMAC_BLOCK_SIZE * EMAC_TX_BLOCK_NUM) 


/* EMAC Descriptor TX and RX Control fields */
#define EMAC_TX_DESC_INT        0x80000000
#define EMAC_TX_DESC_LAST       0x40000000
#define EMAC_TX_DESC_CRC        0x20000000
#define EMAC_TX_DESC_PAD        0x10000000
#define EMAC_TX_DESC_HUGE       0x08000000
#define EMAC_TX_DESC_OVERRIDE   0x04000000

#define EMAC_RX_DESC_INT        0x80000000

/* EMAC Descriptor status related definition */
#define TX_DESC_STATUS_ERR      0x80000000
#define TX_DESC_STATUS_NODESC   0x40000000
#define TX_DESC_STATUS_UNDERRUN 0x20000000
#define TX_DESC_STATUS_LCOL     0x10000000
#define TX_DESC_STATUS_ECOL     0x08000000
#define TX_DESC_STATUS_EDEFER   0x04000000
#define TX_DESC_STATUS_DEFER    0x02000000
#define TX_DESC_STATUS_COLCNT   0x01E00000  /* four bits, it's a mask, not exact count */

#define RX_DESC_STATUS_ERR      0x80000000
#define RX_DESC_STATUS_LAST     0x40000000
#define RX_DESC_STATUS_NODESC   0x20000000
#define RX_DESC_STATUS_OVERRUN  0x10000000
#define RX_DESC_STATUS_ALGNERR  0x08000000
#define RX_DESC_STATUS_RNGERR   0x04000000
#define RX_DESC_STATUS_LENERR   0x02000000
#define RX_DESC_STATUS_SYMERR   0x01000000
#define RX_DESC_STATUS_CRCERR   0x00800000
#define RX_DESC_STATUS_BCAST    0x00400000
#define RX_DESC_STATUS_MCAST    0x00200000
#define RX_DESC_STATUS_FAILFLT  0x00100000
#define RX_DESC_STATUS_VLAN     0x00080000
#define RX_DESC_STATUS_CTLFRAM  0x00040000

#define DESC_SIZE_MASK          0x000007FF  /* 11 bits for both TX and RX */

/* EMAC interrupt controller related definition */
#define EMAC_INT_RXOVERRUN  0x01 << 0
#define EMAC_INT_RXERROR    0x01 << 1 
#define EMAC_INT_RXFINISHED 0x01 << 2
#define EMAC_INT_RXDONE     0x01 << 3 
#define EMAC_INT_TXUNDERRUN 0x01 << 4
#define EMAC_INT_TXERROR    0x01 << 5 
#define EMAC_INT_TXFINISHED 0x01 << 6
#define EMAC_INT_TXDONE     0x01 << 7 
#define EMAC_INT_SOFTINT    0x01 << 12
#define EMAC_INT_WOL        0x01 << 13 


#endif

