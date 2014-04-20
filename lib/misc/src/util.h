#ifndef UTILS_H
#define UTILS_H

#include <types.h>

void util_sdelay(uint32 v);
uint32 util_rand();
void util_rand_seed(uint32 seed);
uint8 util_swapb(uint8 b);
uint8 util_dec2bcd(uint8 dec);

#define UTIL_RAND_MAX 32767

#define BITMASK_CLEAR(bitset, bitmask) (bitset &= ~(bitmask))
#define BITMASK_SET(bitset, bitmask)   (bitset |= (bitmask))

#define BIT_CLEAR(bitset, bit) (bitset &= ~(1 << bit))
#define BIT_SET(bitset, bit)   (bitset |= (1 << bit))

#define MABS(value) (value > 0 ? value : ((-1) * value))

#endif

