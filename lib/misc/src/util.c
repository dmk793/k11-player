#include "util.h"

static uint32 next;

/*
 * simple delay
 * NOTE optimization should be disabled
 */
void __attribute__((optimize("O0"))) util_sdelay(uint32 v)
{
    while (v) v--;
}

/*
 * RAND_MAX assumed to be 32767
 */
uint32 util_rand() {
    next = next * 1103515245 + 12345;
    return((unsigned)(next/65536) % 32768);
}

/*
 *
 */
void util_rand_seed(uint32 seed) {
    next = seed;
}

/*
 * swap bits in byte
 */
uint8 util_swapb(uint8 b) 
{
    b = ((b & 0xaa) >> 1) | ((b & 0x55) << 1);
    b = ((b & 0xcc) >> 2) | ((b & 0x33) << 2);
    return (((b >> 4) & 0x0f) | ((b << 4) & 0xf0));
}


/*
 * convert decimal number to BCD representation
 *
 * NOTE max number is 99
 */
uint8 util_dec2bcd(uint8 dec)
{
    return ((dec / 10 * 16) | (dec % 10));
}


