/*
 * NOTE this functions should be used atomically, be care in OS invironment
 *
 * CCITT: x^16 + x^15 + x^5 + 1
 * CRC16: x^16 + x^15 + x^2 + 1
 * CRC32: x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8 + x^7 + x^5 + x^4 + + x ^2 + x + 1
 */
#include <LPC177x_8x.h>
#include "hwcrc.h"

/*
 *
 */
void hwcrc_ccitt_init(uint16 *crc)
{
    *crc = 0xffff;

    LPC_CRC->MODE = 0x00000000;
    LPC_CRC->SEED = 0xffff;
}

/*
 *
 */
void hwcrc_ccitt(uint16 *crc, uint8* buf, int len)
{
    while (len--)
        LPC_CRC->WR_DATA_BYTE = *buf++;

    *crc = LPC_CRC->SUM & 0xffff;
}

/*
 *
 */
void hwcrc_16_init(uint16 *crc)
{
    *crc = 0x0000;

    LPC_CRC->MODE = 0x00000015;
    LPC_CRC->SEED = 0x0000;
}

/*
 *
 */
void hwcrc_16(uint16 *crc, uint8* buf, int len)
{
    while (len--)
        LPC_CRC->WR_DATA_BYTE = *buf++;

    *crc = LPC_CRC->SUM & 0xffff;
}

/*
 *
 */
void hwcrc_32_init(uint32 *crc)
{
    *crc = 0xffffffff;

    LPC_CRC->MODE = 0x00000036;
    LPC_CRC->SEED = 0xffffffff;
}

/*
 *
 */
void hwcrc_32(uint32 *crc, uint8* buf, int len)
{
    while (len--)
        LPC_CRC->WR_DATA_BYTE = *buf++;

    *crc = LPC_CRC->SUM;
}

