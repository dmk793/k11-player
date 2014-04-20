#ifndef HWCRC_LPC17XX_H
#define HWCRC_LPC17XX_H

#include <types.h>

void hwcrc_ccitt_init(uint16 *crc);
void hwcrc_ccitt(uint16 *crc, uint8* buf, int len);
void hwcrc_16_init(uint16 *crc);
void hwcrc_16(uint16 *crc, uint8* buf, int len);
void hwcrc_32_init(uint32 *crc);
void hwcrc_32(uint32 *crc, uint8* buf, int len);

#endif

