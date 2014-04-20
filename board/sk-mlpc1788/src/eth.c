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
 *
 */
#include <string.h>
#include <LPC177x_8x.h>
#include <LPC177x_8x_bits.h>
#include <debug.h>
#include <util.h>
#include <stimer.h>
#include "os.h"
#include "osw_objects.h"
#include "irqp.h"
#include "eth.h"
#include "eth_def.h"
#include "net/net.h"
#include "net/net_def.h"

struct ethernet_t ethernet;
//struct net_eth_frame_t ethframe; /* NOTE large part of RAM */

static void phy_write(uint8 addr, uint8 reg, uint16 value);
static void phy_read(uint8 addr, uint8 reg, uint16 *value);

static int eth_start();
static int eth_an();
static int phy_init();
static void emacTxDescriptorInit();
static void emacRxDescriptorInit();
static void emac_rx();
//static void emac_tx();
static int emac_start_xmit(void* buf, int length);

#define reg_write(reg, data) ((*(volatile unsigned long *)(reg)) = (volatile unsigned long)(data))
#define reg_read(reg) (*(volatile unsigned long *)(reg))

/*
 *
 */
int eth_init()
{
    ethernet.link = 0;
    stimer_settime(&ethernet.to);

    {
        int i;

        /*
         * PHYAD0       input with internal pull-up during reset
         * PHYAD[1:4]   input with internal pull-down during reset
         */
        ethernet.phyaddr = 0x01;

        for (i = 0; i < 6; i++)
            ethernet.macaddr[i] = net_info.haddr[i];
    }

    LPC_SC->PCONP |= PCONP_PCENET;

    /*
     * P1.0     ENET_TXD0
     * P1.1     ENET_TXD1
     * P1.4     ENET_TX_EN
     * P1.8     ENET_CRS
     * P1.9     ENET_RXD0
     * P1.10    ENET_RXD1
     * P1.14    ENET_RX_ER
     * P1.15    ENET_REF_CLK
     * P1.16    ENET_MDC
     * P1.17    ENET_MDIO
     */
    LPC_IOCON->P1_0  = 0x21;
    LPC_IOCON->P1_1  = 0x21;
    LPC_IOCON->P1_4  = 0x21;
    LPC_IOCON->P1_8  = 0x21;
    LPC_IOCON->P1_9  = 0x21;
    LPC_IOCON->P1_10 = 0x21;
    LPC_IOCON->P1_14 = 0x21;
    LPC_IOCON->P1_15 = 0x21;
    LPC_IOCON->P1_16 = 0x21;
    LPC_IOCON->P1_17 = 0x21;

    return eth_start();
}

/*
 *
 */
int eth_linkup()
{
    uint16 reg;
#define ETHERNET_CHECK_ENERGY_TO       1000
#define ETHERNET_CHECK_AN_TO           50
#define ETHERNET_CHECK_AN_RETRY_COUNT  100

    if (stimer_deltatime(ethernet.to) < ETHERNET_CHECK_ENERGY_TO)
        return ethernet.link;
    stimer_settime(&ethernet.to);

    phy_read(ethernet.phyaddr, PHY_KS8721BL_100BASE_PHYCTRL, &reg);
    if (reg & PHY_KS8721BL_100BASE_PHYCTRL_ENRG_DETECT)
    {
        if (ethernet.link == 0)
        {
            dprint("sn", "ethernet, energy detect");
            dprint("s", "ethernet, started auto negotiation  ");

            /* auto negotiation */
            {
                ethernet.anretry = ETHERNET_CHECK_AN_RETRY_COUNT;

                phy_write(ethernet.phyaddr, PHY_KS8721BL_BMCR,
                        PHY_KS8721BL_BMCR_AN | PHY_KS8721BL_BMCR_RE_AN);

                while (ethernet.anretry--) 
                {
                    dprint("s", "\b \b");
                    if (eth_an())
                    {
                        ethernet.link = 1;
                        goto out;
                    }

                    switch (ethernet.anretry % 4)
                    {
                        case 3:  debug_char('|'); break;
                        case 2:  debug_char('/'); break;
                        case 1:  debug_char('-'); break;
                        case 0:  debug_char('\\'); break;
                        default: debug_char(' '); break;
                    }

                    os_wait_ms(ETHERNET_CHECK_AN_TO);
                }

                ethernet.anretry++;
                dprint("sn", "\bNO LINK");
                ethernet.state = ETHERNET_STATE_IDLE;
            }
        }
    } else {
        if (ethernet.link)
            dprint("sn", "ethernet, link lost");
        ethernet.link = 0;
    }

out:
    return ethernet.link;

//
//    Ethernet_Handler();
//
}

/*
 *
 */
static int eth_start()
{
    /* reset MAC modules, tx, mcs_tx, rx, mcs_rx, simulation and soft reset */
    LPC_EMAC->MAC1 = EMAC_MAC1_RESET_TX         |
                     EMAC_MAC1_RESET_MCS_TX     |
                     EMAC_MAC1_RESET_RX         |
                     EMAC_MAC1_RESET_MCS_RX     |
                     EMAC_MAC1_SIMULATION_RESET |
                     EMAC_MAC1_SOFT_RESET;

    /* reset datapaths and host registers */
    LPC_EMAC->Command = EMAC_COMMAND_REG_RESET |
                        EMAC_COMMAND_TX_RESET  |
                        EMAC_COMMAND_RX_RESET;

    /* short delay after reset */
    util_sdelay(100); /* short delay */

    LPC_EMAC->MAC1 = 0;

    /* disable TX */
    LPC_EMAC->Command &= ~EMAC_COMMAND_TX_ENABLE;
    /* disable RX */
    LPC_EMAC->Command &= ~EMAC_COMMAND_RX_ENABLE;

    /*
     * initialize MAC2 to default value
     * NOTE don't think this is necessary??  m_nic_write(EMAC_MAC2, 0x0
     */
    LPC_EMAC->MAC2 = 0;

    /*
     * non back-to-back Inter-Packet-Gap register
     * The manual recommends the value 0x12
     */
    LPC_EMAC->IPGR = 0x12;

    /* collision window/retry register. Using recommended value from manual. */
    LPC_EMAC->CLRT = 0x370F;

    /*
     * maximum frame register. 
     * NOTE the default reset value is already 0x0600!!  m_nic_write(EMAC_MAXF, 0x0600);
     */

    /* intialize PHY. emacSpeed and emacDuplex will be set in phyInit */
#define MAX_PHY_INIT_RETRY 10
    {
        uint8 i;

        for (i = 0; i < MAX_PHY_INIT_RETRY; i++)
        {
            if (phy_init())
                break;
        }
        if (i == MAX_PHY_INIT_RETRY)
        {
            dprint("sn", ERR_PREFIX "Ethernet init error");
            return 0;
        }
    }

    /* write the mac address */
    LPC_EMAC->SA0 = (ethernet.macaddr[5] << 8 | ethernet.macaddr[4]);
    LPC_EMAC->SA1 = (ethernet.macaddr[3] << 8 | ethernet.macaddr[2]);
    LPC_EMAC->SA2 = (ethernet.macaddr[1] << 8 | ethernet.macaddr[0]);

    dprint("s1d*:1d*:1d*:1d*:1d*:1dn", "MAC ",
            ethernet.macaddr[0],
            ethernet.macaddr[1],
            ethernet.macaddr[2],
            ethernet.macaddr[3],
            ethernet.macaddr[4],
            ethernet.macaddr[5]);
    
    emacTxDescriptorInit();
    emacRxDescriptorInit();

    /* pass all receive frames */
    BITMASK_SET(LPC_EMAC->MAC1, EMAC_MAC1_PASS_ALL_RECEIVE_FRAMES);

    /*
     * set up the Rx filter 
     * [0]-AllUnicast, [1]-AllBroadCast, [2]-AllMulticast, [3]-UnicastHash
     * [4]-MulticastHash, [5]-Perfect, [12]-MagicPacketEnWoL, [13]-RxFilterEnWoL
     */
    LPC_EMAC->RxFilterCtrl = 0x0022;

    /* clear all interrupts */
    LPC_EMAC->IntClear = 0xffff;

    /* TODO the irq is installed here in sample code */

    /* enable interrupts (not SoftInt and WoL) */
    LPC_EMAC->IntEnable = 0x00ff;

    /* enable Rx & Tx */
    {
        BITMASK_SET(LPC_EMAC->Command, EMAC_COMMAND_RX_ENABLE);
        BITMASK_SET(LPC_EMAC->Command, EMAC_COMMAND_TX_ENABLE);
        BITMASK_SET(LPC_EMAC->MAC1, EMAC_MAC1_RECEIVE_ENABLE);
    }

    NVIC_EnableIRQ(ENET_IRQn);
    NVIC_SetPriority(ENET_IRQn, ENET_IRQP);
//    ethernet.link = 1;

    return 1;
}

/*
 *
 */
static int eth_an()
{
    uint16 phyctrl;

//    /* auto negotiation */
//    phy_write(ethernet.phyaddr, PHY_KS8721BL_BMCR,
//            PHY_KS8721BL_BMCR_AN | PHY_KS8721BL_BMCR_RE_AN);

    phy_read(ethernet.phyaddr, PHY_KS8721BL_100BASE_PHYCTRL, &phyctrl);

    if (!(((phyctrl & PHY_KS8721BL_100BASE_PHYCTRL_OPMODE_MASK) != PHY_KS8721BL_100BASE_PHYCTRL_OPMODE_AN) && 
          (phyctrl & PHY_KS8721BL_100BASE_PHYCTRL_AN_COMPLETE)))
    {
        return 0;
    }

    /* link established when AN status bit (bit 7) is set */
    dprint("ns", "ethernet, link ");
    phy_read(ethernet.phyaddr, PHY_KS8721BL_100BASE_PHYCTRL, &phyctrl);

    /* check status */
    switch (phyctrl & PHY_KS8721BL_100BASE_PHYCTRL_OPMODE_MASK)
    {
        case PHY_KS8721BL_100BASE_PHYCTRL_OPMODE_10BT_HD:
            ethernet.speed  = SPEED_10;
            ethernet.duplex = HALF_DUPLEX;
            dprint("sn", "10 Mbps, half duplex");
            break;
        case PHY_KS8721BL_100BASE_PHYCTRL_OPMODE_100BT_HD:
            ethernet.speed  = SPEED_100;
            ethernet.duplex = HALF_DUPLEX;
            dprint("sn", "100 Mbps, half duplex");
            break;
        case PHY_KS8721BL_100BASE_PHYCTRL_OPMODE_10BT_FD:
            ethernet.speed  = SPEED_10;
            ethernet.duplex = FULL_DUPLEX;
            dprint("sn", "10 Mbps, full duplex");
            break;
        case PHY_KS8721BL_100BASE_PHYCTRL_OPMODE_100BT_FD:
            ethernet.speed  = SPEED_100;
            ethernet.duplex = FULL_DUPLEX;
            dprint("sn", "100 Mbps, full duplex");
            break;
        default:
            /* NOTREACHED */
            dprint("sn", "unknown");
            return 0;
    }

    /* 10 Mbps and half-duplex */
    if (ethernet.speed == SPEED_10 && ethernet.duplex == HALF_DUPLEX)
    {
        /* CRC and PAD enabled */
        LPC_EMAC->MAC2 = 0x30;

        /* 10 Mbps mode */
        BITMASK_CLEAR(LPC_EMAC->SUPP, EMAC_SUPP_SPEED);

        /* PassRuntFrame and RMII */
        BITMASK_SET(LPC_EMAC->Command,
                EMAC_COMMAND_PASS_RUNT_FRAME | EMAC_COMMAND_RMII);

        /* back-to-back Inter-Packet-Gap register. Recommended value from manual. */
        LPC_EMAC->IPGT = 0x12;
    } else if (ethernet.speed == SPEED_100 && ethernet.duplex == HALF_DUPLEX) {
        /* CRC and PAD enabled */
        LPC_EMAC->MAC2 = 0x30;

        /* 100 Mbps mode */
        BITMASK_SET(LPC_EMAC->SUPP, EMAC_SUPP_SPEED);

        /* PassRuntFrame and RMII */
        BITMASK_SET(LPC_EMAC->Command,
                EMAC_COMMAND_PASS_RUNT_FRAME | EMAC_COMMAND_RMII);

        /* back-to-back Inter-Packet-Gap register. Recommended value from manual. */
        LPC_EMAC->IPGT = 0x12;
  } else if (ethernet.speed == SPEED_10 && ethernet.duplex == FULL_DUPLEX) {
        /* CRC and PAD enabled */
        LPC_EMAC->MAC2 = 0x31;

        /* 10 Mbps mode */
        BITMASK_CLEAR(LPC_EMAC->SUPP, EMAC_SUPP_SPEED);

        /* PassRuntFrame, RMII and Full-Duplex */
        BITMASK_SET(LPC_EMAC->Command,
                EMAC_COMMAND_PASS_RUNT_FRAME | EMAC_COMMAND_RMII | EMAC_COMMAND_FULL_DUPLEX);

        /* back-to-back Inter-Packet-Gap register. Recommended value from manual. */
        LPC_EMAC->IPGT = 0x15;
  } else if (ethernet.speed == SPEED_100 && ethernet.duplex == FULL_DUPLEX) {
        /* CRC and PAD enabled */
        LPC_EMAC->MAC2 = 0x31;

        /* 100 Mbps mode */
        BITMASK_SET(LPC_EMAC->SUPP, EMAC_SUPP_SPEED);

        /* PassRuntFrame, RMII and Full-Duplex */
        BITMASK_SET(LPC_EMAC->Command,
                EMAC_COMMAND_PASS_RUNT_FRAME | EMAC_COMMAND_RMII | EMAC_COMMAND_FULL_DUPLEX);

        /* back-to-back Inter-Packet-Gap register. Recommended value from manual. */
        LPC_EMAC->IPGT = 0x15;
    }

    return 1;
}



/*
 *
 */
static int phy_init()
{
    dprint("sn", "PHY init");

    /* host clock divided by 28, no suppress preamble, no scan increment */
    LPC_EMAC->MCFG = EMAC_MCFG_CLK_DIV28 | EMAC_MCFG_RES_MII;
    util_sdelay(100); /* short delay */

    LPC_EMAC->MCFG = EMAC_MCFG_CLK_DIV28;
    LPC_EMAC->MCMD = 0;
    
    /* RMII configuration */

    /* TODO do we really need to set this again? */

    LPC_EMAC->Command |= EMAC_COMMAND_RMII;

    //m_nic_write(EMAC_SUPP,  0x0900);                 /* RMII setting, PHY support: [8]=0 ->10 Mbps mode, =1 -> 100 Mbps mode */
    LPC_EMAC->SUPP = EMAC_SUPP_SPEED | EMAC_SUPP_RES_RMII;
    util_sdelay(100); /* short delay */
    LPC_EMAC->SUPP = EMAC_SUPP_SPEED;

    util_sdelay(500); /* short delay */

    /* check PHY ID */
    {
        uint16 id1, id2;

        dprint("sn", "Micrel KS8721BL");

        phy_read(ethernet.phyaddr, PHY_KS8721BL_PHYIDR1, &id1);
        phy_read(ethernet.phyaddr, PHY_KS8721BL_PHYIDR2, &id2);

        dprint("s2xn", "    ID1 ", id1);
        dprint("s2xn", "    ID2 ", id2);

        /* 
         * Micrel OUI 0x0010A1 ?
         *
         * ID1
         *     0..15     3..18 bits of OUI
         * ID2
         *      0..3     revision number
         *      4..9     model number
         *     10..15    19..24 bits of OUI
         */
        if (id1 != 0x0022)
        {
            dprint("sn", "phy ID1 mismatch");
            return 0;
        }
        if ((id2 & 0xfff0) != 0x1610)
        {
            dprint("sn","phy ID2 mismatch");
            return 0;
        }

    }

    {
        /* XXX what purpose of so complex reset ? */

        /* software reset of PHY. The bit is self-clearing */
        phy_write(ethernet.phyaddr, PHY_KS8721BL_BMCR, PHY_KS8721BL_BMCR_RESET);
        util_sdelay(100); /* short delay */

        phy_write(ethernet.phyaddr, PHY_KS8721BL_BMCR, PHY_KS8721BL_BMCR_RESET);
        util_sdelay(10000); /* short delay */

        {
            uint32 timeout;
            uint16 bmcr;

            timeout = 0x01000000;
            while (timeout)
            {
                phy_read(ethernet.phyaddr, PHY_KS8721BL_BMCR, &bmcr);

                if (!(bmcr & PHY_KS8721BL_BMCR_RESET))
                    break;
                timeout--;
            }
            if (timeout == 0)
            {
                dprint("sn", "phy reset timeout");
                return 0;
            }
        }
    }

    return 1;
}

/*
 *
 */
static void phy_write(uint8 addr, uint8 reg, uint16 value)
{
    /* Set up address to access in MII Mgmt Address Register */
    LPC_EMAC->MADR = (addr << 8) | reg;
    /* Write value into MII Mgmt Write Data Register */
    LPC_EMAC->MWTD = value;
    /* TODO timeout */
    while (1)
    {
        if ((LPC_EMAC->MIND & EMAC_MIND_BUSY) == 0)
            break;
    }
}

/*
 *
 */
static void phy_read(uint8 addr, uint8 reg, uint16 *value)
{
    /* Set up address to access in MII Mgmt Address Register */
    LPC_EMAC->MADR = (addr << 8) | reg;
    /* Trigger a PHY read via MII Mgmt Command Register */
    LPC_EMAC->MCMD = EMAC_MCMD_READ;
    /* TODO timeout */
    while (1)
    {
        if ((LPC_EMAC->MIND & EMAC_MIND_BUSY) == 0)  
        {
            *value = LPC_EMAC->MRDD;
            LPC_EMAC->MCMD = 0; /* Cancel read */
            return;
        }
    }
    *value = 0;
}

/*
 *
 */
static void emacTxDescriptorInit()
{
    int i = 0;
    uint32* txDescAddr   = NULL;
    uint32* txStatusAddr = NULL;

    LPC_EMAC->TxDescriptor = TX_DESCRIPTOR_ADDR;
    LPC_EMAC->TxStatus     = TX_STATUS_ADDR;

    LPC_EMAC->TxDescriptorNumber = EMAC_TX_DESCRIPTOR_COUNT - 1;

    for (i = 0; i < EMAC_TX_DESCRIPTOR_COUNT; i++)
    {
        txDescAddr = (uint32 *)(TX_DESCRIPTOR_ADDR + i * 8);
        reg_write(txDescAddr, (EMAC_TX_BUFFER_ADDR + i * EMAC_BLOCK_SIZE));

        /* control field in descriptor */
        txDescAddr++;
        reg_write(txDescAddr, (EMAC_TX_DESC_INT | (EMAC_BLOCK_SIZE - 1)));
    }

    for (i = 0; i < EMAC_TX_DESCRIPTOR_COUNT; i++)
    {
        txStatusAddr = (uint32 *)(TX_STATUS_ADDR + i * 4);
        reg_write(txStatusAddr, 0);
    }

    LPC_EMAC->TxProduceIndex = 0;
}

/*
 *
 */
static void emacRxDescriptorInit() 
{
    int i;
    uint32* rxDescAddr   = NULL;
    uint32* rxStatusAddr = NULL;

    /* base address of rx descriptor array */
    LPC_EMAC->RxDescriptor = RX_DESCRIPTOR_ADDR;

    /* base address of rx status */
    LPC_EMAC->RxStatus = RX_STATUS_ADDR;

    /* number of rx descriptors */
    LPC_EMAC->RxDescriptorNumber = EMAC_RX_DESCRIPTOR_COUNT - 1;

    for (i = 0; i < EMAC_RX_DESCRIPTOR_COUNT; i++)
    {
        rxDescAddr = (uint32 *)(RX_DESCRIPTOR_ADDR + i * 8);
        reg_write(rxDescAddr, (EMAC_RX_BUFFER_ADDR + i * EMAC_BLOCK_SIZE));

        rxDescAddr++;
        reg_write(rxDescAddr, (EMAC_RX_DESC_INT | ((EMAC_BLOCK_SIZE - 1) & DESC_SIZE_MASK) ) );
    }

    for (i = 0; i < EMAC_RX_DESCRIPTOR_COUNT; i++)
    {
        /* RX status, two words, status info. and status hash CRC. */
        rxStatusAddr = (uint32 *)(RX_STATUS_ADDR + i * 8);  
        reg_write(rxStatusAddr, 0);
        rxStatusAddr++;
        reg_write(rxStatusAddr, 0);
    }

    LPC_EMAC->RxConsumeIndex = 0;
}

/*
 *
 */
void Ethernet_Handler()
{
//    volatile uint32 regValue = 0;
    os_event_raise(&net.events, NET_EVENT_MASK_NET_IRQ);

//    debug_str("-irq\r\n");
    NVIC_DisableIRQ(ENET_IRQn);
}

/*
 *
 */
void eth_handler_bh()
{
    volatile uint32 regValue = 0;

    regValue = LPC_EMAC->IntStatus;

    if (regValue == 0)
        goto out;

    if (regValue & EMAC_INT_RXOVERRUN)
    {
        LPC_EMAC->IntClear = EMAC_INT_RXOVERRUN;
        dprint("sn", "rx overrun");
        goto out;
    }

    if (regValue & EMAC_INT_RXERROR)
    {
        LPC_EMAC->IntClear = EMAC_INT_RXERROR;
//        debug_str("rx error\r\n");
        goto out;
    }

    if (regValue & EMAC_INT_RXFINISHED)
    {
        LPC_EMAC->IntClear = EMAC_INT_RXFINISHED;
//        debug_str("rx finished\r\n");
    }

    if (regValue & EMAC_INT_RXDONE)
    {
        LPC_EMAC->IntClear = EMAC_INT_RXDONE;
        emac_rx();   
    }

    if (regValue & EMAC_INT_TXUNDERRUN)
    {
        LPC_EMAC->IntClear = EMAC_INT_TXUNDERRUN;

        dprint("sn", "tx underrun");
        goto out;
    }

    if (regValue & EMAC_INT_TXERROR)
    {
        LPC_EMAC->IntClear = EMAC_INT_TXERROR;
        dprint("sn", "tx error");
        goto out;
    }

    if (regValue & EMAC_INT_TXFINISHED)
    {
        LPC_EMAC->IntClear = EMAC_INT_TXFINISHED;
    }

    if (regValue & EMAC_INT_TXDONE)
    {
        LPC_EMAC->IntClear = EMAC_INT_TXDONE;
//        os_event_raise(OSW_EVENT_NET);
//        emac_tx();
    }

out:
    NVIC_EnableIRQ(ENET_IRQn);
}

/*
 *
 */
static void emac_rx()
{
    uint32 rxProduceIndex;
    uint32 rxConsumeIndex;
    uint32* rxStatusAddr;
    uint32 recvSize;
    uint32* recvAddr;

    /* the input parameter, EMCBuf, needs to be word aligned */

    rxProduceIndex = LPC_EMAC->RxProduceIndex;
    rxConsumeIndex = LPC_EMAC->RxConsumeIndex;

    /* consume the received packets */
    while (rxConsumeIndex != rxProduceIndex)
    {
        rxStatusAddr = (uint32*)(RX_STATUS_ADDR + rxConsumeIndex * 8);
        recvSize = reg_read(rxStatusAddr);

        if ((recvSize & RX_DESC_STATUS_LAST) == 0)
        {
            // TODO: could this occur when EMAC_BLOCK_SIZE == 0x0600?
            dprint("sn", "emac_rx: NOT LAST fragment");
        }

        recvSize = (recvSize & DESC_SIZE_MASK) + 1;

        recvAddr = (uint32*)(RX_DESCRIPTOR_ADDR + rxConsumeIndex * 8);

        /* NOTE frame length sanity check */
        if (recvSize <= sizeof(struct net_eth_frame_t))
            net_receive((struct net_eth_frame_t *)reg_read(recvAddr), recvSize);

        rxConsumeIndex++;
        if (rxConsumeIndex == EMAC_RX_DESCRIPTOR_COUNT)
            rxConsumeIndex = 0;

        LPC_EMAC->RxConsumeIndex = rxConsumeIndex;
    }

    return;
}

/*
 *
 */
void eth_send(uint8 *packet, uint32 len)
{
    if (!len)
        return;

    emac_start_xmit(packet, len);
}

/*
 * check wheather transmitter ready to transmit frame
 *
 * RETURN
 *     0    transmitter not ready to transmit another frame
 *     1    transmitter can transmit frame
 */
int eth_txready()
{
    uint32 txProduceIndex;
    uint32 txConsumeIndex;

    txProduceIndex = LPC_EMAC->TxProduceIndex;
    txConsumeIndex = LPC_EMAC->TxConsumeIndex;

    /* check if descriptor array is full */
    if (txProduceIndex == (txConsumeIndex == 0 ? (EMAC_TX_DESCRIPTOR_COUNT - 1) : (txConsumeIndex - 1)))
    {
        return 0;
    }

    return 1;
}

/*
 *
 */
static int emac_start_xmit(void* buf, int length)
{
    uint32 txProduceIndex = 0;
//    uint32 txConsumeIndex = 0;
    uint8* pData          = 0;
    uint32 len            = length;//skb->len;
    uint32 sendLen        = 0;
    uint32* tx_desc_addr   = NULL;

    txProduceIndex = LPC_EMAC->TxProduceIndex;
//    txConsumeIndex = LPC_EMAC->TxConsumeIndex;

    /* TODO check txready ? */

//    if (txConsumeIndex != txProduceIndex)
//    {
//        // TODO why return here? This just means that the transmit array isn't empty
//        debug_str("emac: emac_tx transmit array isn't empty\r\n");
//        return -1;
//    }

//    if (txProduceIndex == EMAC_TX_DESCRIPTOR_COUNT)
//    {
//        // should never happen
//        // TODO remove
//        debug_str("emac: emac_tx produce index == count\r\n");
//    }

    if (len == 0)
        return 0;

    pData = (uint8*)EMAC_TX_BUFFER_ADDR;
    memcpy(pData, buf, length); 

    /* XXX frame should feet in one packet */
    do
    {
        tx_desc_addr = (uint32*) (TX_DESCRIPTOR_ADDR + txProduceIndex * 8);

        sendLen = len;
        if (sendLen > EMAC_BLOCK_SIZE)
        {
            sendLen = EMAC_BLOCK_SIZE;
        }
        else
        {
            /* last fragment */
            sendLen |= EMAC_TX_DESC_LAST;
        }

        reg_write(tx_desc_addr, (uint32)pData);
        tx_desc_addr++;
        reg_write(tx_desc_addr, (uint32)(EMAC_TX_DESC_INT | (sendLen -1)));

        txProduceIndex++;
        if (txProduceIndex == EMAC_TX_DESCRIPTOR_COUNT)
        {
            txProduceIndex = 0;
        }

        LPC_EMAC->TxProduceIndex = txProduceIndex;

        len   -= (sendLen & ~EMAC_TX_DESC_LAST);
        pData += (sendLen & ~EMAC_TX_DESC_LAST);
    } while (len > 0);

    return 0;
}

